#pragma once
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include "fixed_string.h"

// meta data
// RECORD_FORMAT = '<QQQQIHH256sH64sH564sH256sH'
// RECORD_SIZE = 1188 bytes

namespace data{
    // Force the compiler to use 1-byte alignment (Zero padding)
    #pragma pack(push, 1)
    struct news_record {
        uint64_t id;                 // 8 bytes (Primary Key)
        uint64_t published_on;       // 8 Bytes (Unix timestamp for publish date)    
        uint64_t created_on;         // 8 bytes (Unix timestamp for creation)
        uint64_t expires_at;         // 8 bytes (Unix timestamp for TTL)
        uint32_t views;              // 4 bytes (Metric)
        uint16_t category_id;        // 2 byte (Enum mapping)
        uint16_t  flags;             // 2 byte  (Bitmask: 0x01 = deleted, 0x02 = hot, etc.)
        
        fixed_string<256> title;       // 258 bytes (64 char + 2 byte uint16_t length)
        fixed_string<64> author;       // 66 bytes
        fixed_string<564> link;        // 566 bytes
        fixed_string<256> summary;     // 258 bytes
    };
    // Restore default compiler alignment
    #pragma pack(pop)

    // 12 Clean Master Categories (ID -> Name)
    inline const std::unordered_map<std::uint16_t, std::string> CATEGORY_NAMES = {
        {0, "General"},
        {1, "World News"},
        {2, "Politics"},
        {3, "Business & Finance"},
        {4, "Technology"},
        {5, "Science"},
        {6, "Health"},
        {7, "Sports"},
        {8, "Entertainment & Arts"},
        {9, "Environment"},
        {10, "Crime & Law"},
        {11, "Lifestyle"}
    };
    inline std::string get_category_name(std::uint16_t id) {
        auto it = CATEGORY_NAMES.find(id);
        if (it != CATEGORY_NAMES.end()) {
            return it->second;
        }
        return "General";
    }

}