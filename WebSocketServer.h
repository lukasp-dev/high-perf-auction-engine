#pragma once
#include <uWebSockets/App.h>
#include <iostream>
#include <string_view>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include "AuctionEngine.h"
#include "Parser.h"

namespace Auction {

struct ConnectionData {
    std::string client_id;
};

class WebSocketServer {
public:
    explicit WebSocketServer(AuctionEngine& engine, int port = 9001)
        : engine_(engine), port_(port), app_(nullptr) {}

    void run() {
        std::cout << "[WebSocket] Starting server on port " << port_ << "..." << std::endl;

        uWS::App app;

        app.ws<ConnectionData>("/*", {
            .compression = uWS::SHARED_COMPRESSOR,
            .maxPayloadLength = 16 * 1024,
            .idleTimeout = 30,
            .open = [this](auto* ws, auto* req) {
                std::lock_guard lock(clients_mutex_);
                clients_.insert(ws);
                std::cout << "[WebSocket] Client connected. Total: " << clients_.size() << std::endl;
            },
            .message = [this, &app](auto* ws, std::string_view message, uWS::OpCode opCode) {
                handleMessage(ws, message);
            },
            .close = [this](auto* ws, int code, std::string_view message) {
                std::lock_guard lock(clients_mutex_);
                clients_.erase(ws);
                std::cout << "[WebSocket] Client disconnected. Total: " << clients_.size() << std::endl;
            }
        });

        app.listen(port_, [this](auto* token) {
            if (token) {
                std::cout << "[WebSocket] Server listening on port " << port_ << std::endl;
            } else {
                std::cerr << "[WebSocket] Failed to bind to port " << port_ << std::endl;
            }
        });

        app.run();
    }

    void broadcastTrade(const TradeEvent& event) {
        std::shared_lock lock(clients_mutex_);
        if (clients_.empty()) return;

        std::string message = "{\"type\":\"trade\",\"artwork_id\":\"";
        message += event.artwork_id;
        message += "\",\"winner_id\":\"";
        message += event.winner_id;
        message += "\",\"price\":";
        message += std::to_string(event.price);
        message += ",\"timestamp\":";
        message += std::to_string(event.timestamp);
        message += "}";

        for (auto* ws : clients_) {
            ws->send(message, uWS::OpCode::TEXT);
        }
    }

private:
    void handleMessage(void* ws, std::string_view message) {
        if (message == "ping") {
            static_cast<uWS::WebSocket<false, true, ConnectionData>*>(ws)->send("pong", uWS::OpCode::TEXT);
            return;
        }

        if (message.size() < 10 || message[0] != '{') {
            static_cast<uWS::WebSocket<false, true, ConnectionData>*>(ws)->send(
                "{\"error\":\"invalid format\"}", uWS::OpCode::TEXT);
            return;
        }

        auto bid = parseJsonBid(message);
        if (!bid) {
            static_cast<uWS::WebSocket<false, true, ConnectionData>*>(ws)->send(
                "{\"error\":\"parse error\"}", uWS::OpCode::TEXT);
            return;
        }

        bool is_new_high = engine_.processBid(*bid);

        std::string response = "{\"type\":\"bid_accepted\",\"artwork_id\":\"";
        response += bid->artwork_id;
        response += "\",\"price\":";
        response += std::to_string(bid->price);
        response += ",\"new_high\":";
        response += is_new_high ? "true" : "false";
        response += "}";

        static_cast<uWS::WebSocket<false, true, ConnectionData>*>(ws)->send(response, uWS::OpCode::TEXT);
    }

    std::optional<Bid> parseJsonBid(std::string_view json) {
        Bid bid;
        bool has_artwork = false, has_bidder = false, has_price = false, has_timestamp = false;

        size_t i = 1;
        while (i < json.size()) {
            if (json[i] == '}') break;
            
            size_t key_start = i;
            while (i < json.size() && json[i] != ':') i++;
            if (i >= json.size()) break;
            
            std::string_view key = json.substr(key_start, i - key_start - 1);
            if (key.size() > 1 && key.front() == '"') key = key.substr(1);
            if (key.size() > 1 && key.back() == '"') key = key.substr(0, key.size() - 1);
            
            i++;
            size_t value_start = i;
            if (json[value_start] == '"') {
                value_start++;
                while (i < json.size() && json[i] != '"') i++;
                std::string_view value = json.substr(value_start, i - value_start);
                i++;
                
                if (key == "artwork_id") { bid.artwork_id = value; has_artwork = true; }
                else if (key == "bidder_id") { bid.bidder_id = value; has_bidder = true; }
            } else {
                while (i < json.size() && json[i] != ',' && json[i] != '}') i++;
                std::string_view value = json.substr(value_start, i - value_start);
                
                if (key == "price") {
                    bid.price = Parser::fast_atof(value);
                    has_price = true;
                } else if (key == "timestamp") {
                    auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), bid.timestamp);
                    if (ec == std::errc{}) has_timestamp = true;
                }
            }
            if (json[i] == ',') i++;
        }

        if (!has_timestamp) {
            bid.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }

        if (has_artwork && has_bidder && has_price) return bid;
        return std::nullopt;
    }

    AuctionEngine& engine_;
    int port_;
    void* app_;
    
    std::unordered_set<void*> clients_;
    std::shared_mutex clients_mutex_;
};

}