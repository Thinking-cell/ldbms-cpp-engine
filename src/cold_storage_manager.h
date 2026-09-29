#pragma once
#include<ctime>
#include<cstdint>
#include<filesystem>
#include<fstream>
#include<stdexcept>
#include<unordered_map>
#include<vector>
#include "news_record.h"
#include "cold_data_types.h"
#include "io_metadata_manager.h"

namespace engine{
    class cold_storage_manager{
        const std::uint16_t RECORD_SIZE = sizeof(data::news_record);
        const std::uint32_t BUFFER_SIZE = 50000;
        std::filesystem::path archive_dir_path;
        std::unordered_map<std::uint32_t, std::unique_ptr<monthly_archive>> archives;

        std::uint32_t get_year_month(std::uint64_t timestamp){
            std::time_t time = static_cast<time_t>(timestamp);
            std::tm *time_info = std::gmtime(&time); // learn gmtime
            std::uint32_t year = 1900 + time_info->tm_year; // year is from 1900
            std::uint32_t month = time_info->tm_mon;
            return (year * 100) + month; // yyyymm 
        }

        bool fill_archive_holes(std::vector<data::news_record>  bucket, monthly_archive& archive, std::fstream& rw_stream){
            if(archive.holes.empty()) return true;
            std::vector<std::uint64_t> rollback_holes;
            int initial_hole_index = archive.holes.empty();
            // start filling holes
            while(!bucket.empty() && !archive.holes.empty()){
                data::news_record record = bucket.back();
                std::uint64_t byte_offset = archive.holes.back();
                rw_stream.seekp(byte_offset, std::ios::beg);
                rw_stream.write(reinterpret_cast<char *>(&record), sizeof(record));
                // rollback
                if(!rw_stream.good()){

                }
            }
            
        }

        bool write_archive(std::vector<data::news_record>  bucket, monthly_archive& archive){
            if(bucket.empty()) return true;
            std::fstream rw_stream;
            rw_stream.open(archive.bin_path, std::ios::in | std::ios::out | std::ios::binary);
            if(!rw_stream.is_open()) return false;

            if(!fill_archive_holes(bucket, archive, rw_stream)) return false; 




        }

        
    public:
        cold_storage_manager(std::filesystem::path f_path) : archive_dir_path(f_path){}

        cold_storage_manager(const cold_storage_manager&) = delete;
        cold_storage_manager(cold_storage_manager&&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&&) = delete;

        monthly_archive* get_archive(std::uint32_t ym){
            auto itr = archives.find(ym);
            if(itr == archives.end()) return nullptr;
            return itr->second.get(); // returning pointer to object
        }

        bool create_archive(std::uint32_t ym){
            if(get_archive(ym)) return true; // if archive already exists

            std::filesystem::path dir_path = archive_dir_path / metadata::io_metadata::ARCHIVE_DIRECTORY_NAME;

            metadata::io_metadata_manager::create_directory(dir_path); // create directory if doesnt exist

            std::filesystem::path full_path = dir_path.string() + (std::to_string(ym) + metadata::io_metadata::ARCHIVE_EXT);

            //create file if doesnt exist in append mode in order not to truncate the file if it exists
            std::fstream touch_file;
            touch_file.open(full_path, std::ios::app | std::ios::binary);
            if (!touch_file.good()) return false;
            //create archive and add it to archives
            archives[ym] = std::make_unique<monthly_archive>(ym, full_path);
            return true;
        }

        
        bool backup_records(const std::vector<data::news_record> records){
            if(records.empty()) return true;
            // loop  through records and sort in the appropriate buckets of yyyymm
            std::unordered_map<std::uint32_t, std::vector<data::news_record>> buckets;
            for(const data::news_record& record : records){
                std::uint32_t ym = get_year_month(record.published_on);
                buckets[ym].push_back(record);
            }
            // now access archive for each bucket
            for(auto&[ym, bucket] : buckets){

                create_archive(ym);
                monthly_archive *archive = get_archive(ym);
                if(!archive) continue;
                
                if(!write_archive(bucket, *archive)) continue;

            }

            return true;

        }

    };
}



