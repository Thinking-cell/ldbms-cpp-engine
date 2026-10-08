#pragma once
#include<cstdint>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<vector>

#include "../../core/news_record.h"
#include "io_metadata_manager.h"


namespace injector{
    class record_injector{
        std::filesystem::path src_path;
        const std::uint64_t CHUNK_SIZE;
        const std::uint32_t RECORD_SIZE;

        template<typename ID_GEN_TYPE>
        void id_records(ID_GEN_TYPE get_id, std::vector<data::news_record>& records){
            for(auto& record : records){
                record.id = get_id();
            }
        }

        bool check_path(std::filesystem::path b_path){
            // check if the file actually exists
            if(!std::filesystem::is_regular_file(b_path)) return false;
            // return false if the file has wrong extension
            if(b_path.extension() != metadata::io_metadata::ARCHIVE_EXT) return false;
            return true;

        }
    
    public:
        // default constructor
        record_injector(int c_size = 50000, int r_size = sizeof(data::news_record)) : CHUNK_SIZE(c_size), RECORD_SIZE(r_size){}
        // P constructor
        record_injector(std::filesystem::path bin_path, int c_size = 50000, int r_size = sizeof(data::news_record)) : src_path(bin_path), 
        CHUNK_SIZE(c_size), RECORD_SIZE(r_size){}

       
     
        bool is_path_valid(){
            return check_path(src_path);
        }

        bool set_path(std::filesystem::path b_path){
            
            if(!check_path(b_path)) return false;
            src_path = b_path;
            return true;
        }

        template<typename VEIN, typename ID_GEN_TYPE>
        bool inject_records(VEIN record_vein, ID_GEN_TYPE get_id){
            
            std::ifstream r_stream(src_path, std::ios::binary);
            // return if file doesnt exist
            if(!r_stream.is_open()) return false;
            std::vector<data::news_record> pay_load;

            std::uint64_t record_count = 0;
            
            while(true){
                pay_load.resize(CHUNK_SIZE);
                // read chunk
                r_stream.read(reinterpret_cast<char*>(pay_load.data()), CHUNK_SIZE * RECORD_SIZE);
                std::streamsize red_bytes = r_stream.gcount(); // get the number of bytes red
                if(red_bytes == 0) break; // stop if no bytes are incoming
                if(red_bytes % RECORD_SIZE != 0) return false; // return false if memory footprint is wrong
                if(red_bytes < CHUNK_SIZE * RECORD_SIZE) pay_load.resize(red_bytes / RECORD_SIZE); // resize payload if the red items are less than chunk size
                // return on read failure only for the whole chunk read
                // just continue if the it is the last chunk
                if(!r_stream.good() && red_bytes >= CHUNK_SIZE * RECORD_SIZE) return false; 
                
                id_records(get_id ,pay_load); // generate ids for the records

                record_vein(pay_load); // inject the records in the CSM

                record_count += red_bytes / RECORD_SIZE;

                std::cout << "\x1B[2J\x1B[3J\x1B[H" << std::flush;
                std::cout << "Records Injected: " << std::to_string(record_count) << '\n';
            
                pay_load.clear(); // clear the pay load for the next chunk

            }

            return true;

        }

    };
}