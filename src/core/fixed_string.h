#pragma once
#include<cstring>
#include<cstdint>
#include<iostream>
#include<string_view>


namespace data{
    #pragma pack(push, 1) 
    template<std::uint16_t MAX_LENGTH>
    struct fixed_string{
        char data[MAX_LENGTH];
        std::uint16_t length;

        fixed_string() : length(0){
            std::memset(data, 0, MAX_LENGTH); // instantly replaces all bytes with 0s
            data[0] = '\0';
        }

        fixed_string(const char *s) : length(0){
            if(s == nullptr){
                data[0] = '\0';
                length = 0;
                return;
            }
            // get size of s
            std::uint32_t s_len = std::strlen(s);
            // capping length to max length
            length = (s_len < MAX_LENGTH - 1) ? s_len : (MAX_LENGTH - 1);
            
            // hardware-accelerated full block copy
            std::memcpy(data, s, length);
            data[length] = '\0';
            
            // zero out the remaining memory for safe Disk writes using pointer arithematic
            // in one go
            std::memset(data + length + 1, 0, MAX_LENGTH - length - 1);

        }

        

        

        friend std::ostream& operator<<(std::ostream& o, const fixed_string& s){
            // pushing the entire string into the output stream at once
            return o << std::string_view(s.data, s.length);
        }

        char& operator[](const int index){ return data[index]; }
        const char operator[](int index) const { return data[index]; }


        friend bool operator<(const fixed_string& lhs, const fixed_string& rhs){
            // Use standard lexicographical compare for the smaller length
            // get min length of the two string objects
            std::uint16_t min_len = (lhs.length < rhs.length) ? lhs.length : rhs.length;
            // compare the two blocks of strings till min_len
            int cmp = std::memcmp(lhs.data, rhs.data, min_len);
            // if strings till min_len are not equal then return the 
            // cmp result after converting it to bool
            if (cmp != 0) { return cmp < 0; }
            // if substrings were equal then return bool
            // based on length
            return lhs.length < rhs.length; 
        }

        friend bool operator>(const fixed_string& lhs, const fixed_string& rhs){
            // Use standard lexicographical compare for the smaller length
            // get min length of the two string objects
            std::uint16_t min_len = (lhs.length < rhs.length) ? lhs.length : rhs.length;
            // compare the two blocks of strings till min_len
            int cmp = std::memcmp(lhs.data, rhs.data, min_len);
            // if strings till min_len are not equal then return the 
            // cmp result after converting it to bool
            if (cmp != 0) { return cmp > 0; }
            // if substrings were equal then return bool
            // based on length
            return lhs.length > rhs.length;
        }
        friend bool operator==(const fixed_string& lhs, const fixed_string& rhs){
            if (lhs.length != rhs.length) return false;
            // hardware accelerated full block comparison
            return std::memcmp(lhs.data, rhs.data, lhs.length) == 0; 
        }
        friend bool operator<=(const fixed_string& lhs, const fixed_string& rhs){
            return (lhs < rhs || lhs == rhs); 
        }
        friend bool operator>=(const fixed_string& lhs, const fixed_string& rhs){
            return (lhs > rhs || lhs == rhs); 
        }
    };
    #pragma pack(pop)
}

