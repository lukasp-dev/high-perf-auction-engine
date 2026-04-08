#pragma once
#include "Types.h" 
#include <charconv>     // For std::from_chars (The fastest way to parse numbers)
#include <string_view>  // For Zero-Copy string handling
#include <optional>     // To handle "maybe" results without exceptions

namespace Auction {

class Parser {
public:
    /**
     * @brief [HFT optimization] Fast, zero-allocation float-to-double converter.
     * Needed because Apple Clang doesn't support std::from_chars<double> yet.
     */
    static double fast_atof(std::string_view sv) {
        double val = 0.0;
        double factor = 1.0;
        bool decimal = false;
        for (char c : sv) {
            if (c == '.') {
                decimal = true;
                continue;
            }
            if (!decimal) {
                val = val * 10.0 + (c - '0');
            } else {
                factor *= 0.1;
                val += (c - '0') * factor;
            }
        }
        return val;
    }

    /**
     * @brief Parses a CSV line into a Bid struct.
     * Format: artwork_id,bidder_id,price,timestamp
     * This is "Zero-Copy" because we never create a new std::string.
     */
    static std::optional<Bid> parseBid(std::string_view line) {
        Bid bid;
        
        // STEP 1 -- Find Artwork ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            bid.artwork_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 2 -- Find Bidder ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            bid.bidder_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 3 -- Parse the Price (double) --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            std::string_view price_str = line.substr(0, pos);
            // Use fast_atof instead of std::from_chars for Apple Silicon compatibility
            bid.price = fast_atof(price_str);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 4 -- Parse Timestamp (uint64_t) --
        auto [ptr, ec] = std::from_chars(line.data(),
                                         line.data() + line.size(),
                                         bid.timestamp);

        if (ec != std::errc{}) return std::nullopt;

        return bid;
    }

    /**
     * @brief Parses a CSV line into an Artwork struct.
     * Format: artwork_id,artist_id,royalty,base_price
     */
    static std::optional<Artwork> parseArtwork(std::string_view line) {
        Artwork art;

        // STEP 1 -- Artwork ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            art.artwork_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 2 -- Artist ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            art.artist_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 3 -- Royalty (double) --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            std::string_view royalty_str = line.substr(0, pos);
            // Use fast_atof instead of std::from_chars for Apple Silicon compatibility
            art.royalty = fast_atof(royalty_str);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 4 -- Base Price (uint32_t) --
        auto [ptr, ec] = std::from_chars(line.data(),
                                         line.data() + line.size(),
                                         art.base_price);
        if (ec != std::errc{}) return std::nullopt;

        return art;
    }
};

} // namespace Auction
