#pragma once
#include "Types.h"

namespace Auction {

class OrderBook {
public:
    OrderBook() = default;
    explicit OrderBook(std::string_view id) : artwork_id(id) {}

    /**
     * @brief [Price-Time Priority] Matching Logic
     * Returns true if the new bid becomes the best bid.
     */
    bool handleBid(const Bid& new_bid) {
        // Initial case or higher price
        if (new_bid.price > best_bid.price) {
            best_bid = new_bid;
            return true;
        }
        
        // Same price but earlier timestamp (Price-Time Priority)
        // In our data, later bids usually have later timestamps.
        // best_bid.timestamp == 0 => 첫번 째 입찰자를 1등으로
        if (new_bid.price == best_bid.price && 
            (best_bid.timestamp == 0 || new_bid.timestamp < best_bid.timestamp)) { 
            best_bid = new_bid;
            return true; // if the best bid is renewed, return true;
        }

        return false;
    }

    const Bid& getBestBid() const { return best_bid; }

private:
    std::string_view artwork_id;
    Bid best_bid{}; // Current leader
};

} // namespace Auction
