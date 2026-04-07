#include "Types.h" 
#include <charconv>     // For std::from_chars (The fasteest way to pare numbers)
#include <string_view>  // For Zero-Copy string handling
#include <optional>     // To handle "maybe" results without exceptions

namespace Auction {

class Parser {
public:
    /**
     * @brief Parses a CSV line into a Bid `struct`.
     * This is "Zero-Copy" because we never create a `new` std::string.
     * We just create std::string_view "windows" that point into the original data.
     */
    static std::optional<Bid> parseBid(std::string_view line) {
        Bid bid;
        
        // STEP 1 -- Find Artwork ID --
        if(auto pos = line.find(','); pos != std::string_view::npos) {
            // substr(0, pos) creates a new windown from start to comma
            bid.artwork_id = line.substr(0, pos);
            // `remove_prefix` shifts the start of our main "line" window past the comma.
            line.remove_prefix(pos + 1);
        } else return std::nullptr;

        // STEP 2 -- Find Bidder ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            bid.bidder_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 3 -- Pase the Price --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            std::string_view price_str = line.substr(0, pos);

            auto [ptr, ec] = std::from_chars(price_str.data(),
                            price_str.data() + price_str.size(),
                                                    bid.price);
    
            if (ec != std::errc{}) return std::nullopt;
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // --- STEP 4: Parse Timestamp (uint64_t) ---
        // The remaining 'line' is just the timestamp now.
        auto [ptr, ec] = std::from_chars(line.data(),
                                          line.data() + line.size(),
                                          bid.timestamp);

        if (ec != std::errc{}) return std::nullopt;

        return bid;
     }
    }
}