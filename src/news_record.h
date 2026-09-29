#pragma once
#include <cstdint>
#include "fixed_string.h"

// meta data
// RECORD_FORMAT = '<QQQQIBB4s256sH64sH564sH256sH'
// RECORD_SIZE = 1190 bytes

namespace data{
    // Force the compiler to use 1-byte alignment (Zero padding)
    #pragma pack(push, 1)
    struct news_record {
        uint64_t id;                 // 8 bytes (Primary Key)
        uint64_t published_on;       // 8 Bytes (Unix timestamp for publish date)    
        uint64_t created_on;         // 8 bytes (Unix timestamp for creation)
        uint64_t expires_at;         // 8 bytes (Unix timestamp for TTL)
        uint32_t views;              // 4 bytes (Metric)
        uint8_t category_id;         // 1 byte (Enum mapping)
        uint8_t  flags;              // 1 byte  (Bitmask: 0x01 = deleted, 0x02 = hot, etc.)
        char country_code[4];        // 4 bytes (e.g., "USA\0")
        
        fixed_string<256> title;       // 258 bytes (64 char + 2 byte uint16_t length)
        fixed_string<64> author_email;// 66 bytes
        fixed_string<564> link;       // 566 bytes
        fixed_string<256> summary;    // 258 bytes
    };
    // Restore default compiler alignment
    #pragma pack(pop)
}