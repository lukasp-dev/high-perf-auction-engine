#pragma once
#include "Types.h" 
#include <charconv>     // For std::from_chars (The fastest way to parse numbers)
#include <string_view>  // For Zero-Copy string handling
#include <optional>     // To handle "maybe" results without exceptions

namespace Auction {

class Parser {
public:
    /**
     * @brief Parses a CSV line into a Bid struct.
     * Format: artwork_id,bidder_id,price,timestamp
     * This is "Zero-Copy" because we never create a new std::string.
     * 
     * The objection of this function is to correctly identify and fill values into the Bid struct.
     */
    static std::optional<Bid> parseBid(std::string_view line) {
        Bid bid;
        
        // STEP 1 -- Find Artwork ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) { // 초기화 구문 C++17 부터 도입 if([초기화];[조건식])
            bid.artwork_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 2 -- Find Bidder ID --
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            bid.bidder_id = line.substr(0, pos);
            line.remove_prefix(pos + 1);
        } else return std::nullopt;

        // STEP 3 -- Parse the Price (double) --
        /**
         * 1) 정수형 from_chars() 의 경우
         * [function definition]
         * std::from_chars_result from_chars( const char* first, 
         *                        const char* last, 
         *                        TYPE& value, 
         *                        int base = 10 );
         * 
         * [the return type]
         * struct from_chars_result {
         *       const char* ptr;
         *       std::errc ec;
         * };
         */
        if (auto pos = line.find(','); pos != std::string_view::npos) {
            std::string_view price_str = line.substr(0, pos);
            auto [ptr, ec] = std::from_chars(price_str.data(),                     // character `begin` pointer
                                             price_str.data() + price_str.size(),  // the `end` of the char pointer
                                             bid.price);                           // 세번 째 인자 == 값을 저장할 변수
    
            if (ec != std::errc{}) return std::nullopt;                            // 값 없음(std::nullop)
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
            auto [ptr, ec] = std::from_chars(royalty_str.data(),
                                             royalty_str.data() + royalty_str.size(),
                                             art.royalty);
            if (ec != std::errc{}) return std::nullopt;
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
