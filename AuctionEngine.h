#pragma once
#include <unordered_map>
#include <vector>
#include "OrderBook.h"

namespace Auction {

class AuctionEngine {
public:
    /**
     * @brief Pre-allocates memory to prevent spikes in latency during the hot path.
     */
    void init(size_t expected_bids) {
        // Reserve space for Trade Tape to avoid reallocations.
        // Assuming ~10% of bids will result in a price update.
        trade_tape.reserve(expected_bids / 10);
        
        // Pre-reserve for a reasonable number of artworks to avoid map rehashing.
        books.reserve(50000); 
    }

    /**
     * @brief [The Hot Path] Dispatches a parsed bid to its respective OrderBook.
     * @return true if the bid became the new high bid.
     */
    bool processBid(const Bid& bid) {
        // 1. Dispatch to OrderBook (O(1))
        auto& book = books[bid.artwork_id];
        
        // 2. Matching Logic
        if (book.handleBid(bid)) {
            // 3. Record Trade Event (New Best Bid)
            // Using push_back with {} for reliable aggregate initialization on Apple Clang
            trade_tape.push_back({bid.artwork_id, bid.bidder_id, bid.price, bid.timestamp});
            return true;
        }
        return false;
    }

    const std::vector<TradeEvent>& getTradeTape() const { return trade_tape; }
    size_t getActiveAuctionsCount() const { return books.size(); }

private:
    std::unordered_map<std::string_view, OrderBook> books;
    std::vector<TradeEvent> trade_tape;
};

} // namespace Auction
