#pragma once
#include<algorithm>
#include<ctime>
#include<cstdint>
#include<iomanip>
#include<sstream>
#include<string>
#include<unordered_set>
#include<vector>

/**
 * AI assisted terminal utilities
 */
namespace ui{
    struct parser{
        
        static void trim_spaces(std::string& s){
            // remove_if writes the string's no space characters at the start
            // all the non space characters are accumalated on one side(left)
            // the remove_if returns the iterator at the garbage character
            // reight after the write iterator of the remove_if
            // erase removes all the characters from garbage character to the end of string
            s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        }

        // to parse id list
        static data::td_array<std::uint64_t> parse_id_list(const std::string& input){
            data::td_array<std::uint64_t> result;
            std::stringstream ss(input); // changing it to stream buffer format
            std::string token;
            // iterate over the string extracting tokens(ints)
            // extract all characters before ,
            while(std::getline(ss, token, ',')){
                trim_spaces(token); // strip all spaces
                if(token.empty()) continue; // skip blank character
                try{
                    result.push_back(std::stoull(token));
                }catch(...){
                    continue; // skip in case of invaild token
                }

            }

            return result;
        }

        // parse string to category set
        static std::unordered_set<std::uint16_t> parse_category_set(const std::string& input){
            std::unordered_set<std::uint16_t> result;
            std::stringstream ss(input); 
            std::string token;

            while(std::getline(ss, token, ',')){
                trim_spaces(token); // strip all spaces
                if(token.empty()) continue; // skip blank character
                try{
                    int number = std::stoi(token);
                    if(number < 0 || number > 65535) continue;
                    result.insert(static_cast<std::uint16_t>(number));
                }catch(...){
                    continue; // skip in case of invaild token
                }
            }

            return result;
        }

        // string to unix timestamp parse
        static std::uint64_t parse_date_to_timestamp(const std::string& date_str) {
            std::tm time_struct = {};
            std::stringstream ss(date_str);
            
            // Parses string format exactly as "YYYY-MM-DD"
            ss >> std::get_time(&time_struct, "%Y-%m-%d");
            
            if (ss.fail()) {
                return 0; // Return 0 to indicate a parsing failure
            }
            
            // Convert the populated std::tm struct into a Unix timestamp
            std::time_t unix_time = std::mktime(&time_struct);
            return static_cast<std::uint64_t>(unix_time);
        }
        


    };
}