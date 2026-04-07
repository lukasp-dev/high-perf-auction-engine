#pragma once
#include <string_view>
#include <cstdint>

namespace Auction {

/**
 * [INTERVIEW TIP]
 * Why alignas(64)? 
 * Modern CPUs fetch data in 64-byte chunks (Cache Lines). 
 * If two threads modify different Bids that sit on the same 64-byte line, 
 * they fight over the memory (False Sharing). 
 * Aligning to 64 bytes gives each Bid its own "private lane" in the CPU cache.
 */
struct alignas(64) Bid {
    std::string_view artwork_id;  // Pointer + Length (8+8 bytes)
    std::string_view bidder_id;   // Pointer + Length (8+8 bytes)
    double price{0.0};            // 8 bytes
    uint64_t timestamp{0};        // 8 bytes
    // Total size: 48 bytes. alignas(64) pads this to 64 bytes.
};

// Reference data: Read-mostly. 
// Not aligned to maximize Spatial Locality (fitting more in one cache line).
struct Artwork {
    std::string_view artwork_id;
    std::string_view artist_id;
    double royalty{0.0};
    uint32_t base_price{0};
};

} // namespace Auction
