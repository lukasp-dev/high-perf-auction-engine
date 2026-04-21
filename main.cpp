#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>
#include <atomic>
#include "Parser.h"
#include "AuctionEngine.h"
#include "WebSocketServer.h"

using namespace std;
using namespace Auction;

/**
 * [Lukas Standard] 
 * Full Pipeline Benchmark: Parsing -> Matching -> Recording
 */
int main(int argc, char* argv[]) {
    bool server_mode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--server" || std::string(argv[i]) == "-s") {
            server_mode = true;
        }
    }

    if (server_mode) {
        AuctionEngine engine;
        engine.init(100000);
        
        WebSocketServer server(engine, 9001);
        server.run();
        
        std::cin.get();
        return 0;
    }

    const std::string filename = "bids.csv";
    std::vector<std::string> lines;
    lines.reserve(10000000); // Reserve for 10M entries

    std::cout << "--- Digital Art Auction Engine ---" << std::endl;
    
    // 1. DATA LOADING (Excluded from performance benchmark)
    std::cout << "[1] Loading CSV into memory..." << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: " << filename << " not found! Run 'python3 generate_data.py' first." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    const size_t total_count = lines.size();
    std::cout << "[2] Loaded " << total_count << " entries. Initializing Engine..." << std::endl;

    // Initialize Engine (Pre-allocate memory for the hot path)
    AuctionEngine engine;
    engine.init(total_count);

    // --- 🚀 INTEGRATED BENCHMARK START ---
    // Measures Parsing + Dispatching + Matching + Recording
    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& l : lines) {
        // Step A: Zero-copy Parsing (Parser.h)
        if (auto bid = Parser::parseBid(l)) {
            // Step B & C: Matching & Recording (AuctionEngine.h)
            engine.processBid(*bid);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    // --- 🏁 INTEGRATED BENCHMARK END ---

    // Performance Calculations
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    double seconds = duration_us / 1000000.0;
    double tps = total_count / seconds;
    double avg_latency_ns = (duration_us * 1000.0) / total_count;

    const auto& tape = engine.getTradeTape();

    std::cout << "\n================ Final Performance Report ================" << std::endl;
    std::cout << "Target Hardware      : Apple M4 Pro (ARM64)" << std::endl;
    std::cout << "Total Bids Processed : " << total_count << std::endl;
    std::cout << "Unique Artworks      : " << engine.getActiveAuctionsCount() << std::endl;
    std::cout << "Total Trade Events   : " << tape.size() << " (Price Updates)" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "Total Pipeline Time  : " << std::fixed << std::setprecision(4) << seconds << " s" << std::endl;
    std::cout << "Throughput (TPS)     : " << std::fixed << std::setprecision(0) << tps << " bids/sec" << std::endl;
    std::cout << "Avg Latency          : " << std::fixed << std::setprecision(2) << avg_latency_ns << " ns/bid" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    // --- 📊 TRADE TAPE (Time & Sales) SAMPLE ---
    if (tape.size() >= 5) {
        std::cout << "[Trade Tape Sample - First 5 Price Jumps]" << std::endl;
        for (size_t i = 0; i < 5; ++i) {
            std::cout << "  Trade #" << i + 1 << ": " << tape[i].artwork_id 
                      << " | Winner: " << tape[i].winner_id 
                      << " | Price: $" << std::fixed << std::setprecision(2) << tape[i].price 
                      << " | Time: " << tape[i].timestamp << std::endl;
        }
    }

    return 0;
}
