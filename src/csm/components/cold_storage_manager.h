#pragma once
#include<ctime>
#include<cstdint>
#include<filesystem>
#include<fstream>
#include<stdexcept>
#include<unordered_map>
#include<vector>

#include "../../core/news_record.h"
#include "../../core/cold_data_types.h"
#include "io_metadata_manager.h"
#include "../../sorting/introsort.h"
#include "../../core/cold_data_types.h"
#include "id_gen.h"

namespace engine{
    class cold_storage_manager{

        const std::uint16_t RECORD_SIZE;
        std::filesystem::path archive_dir_path;
        metadata::id_gen_mgr id_gen;
        std::unordered_map<std::uint32_t, std::unique_ptr<monthly_archive>> archives;

        friend class historical_query_engine;

        std::uint32_t get_year_month(std::uint64_t timestamp){
            std::time_t time = static_cast<time_t>(timestamp);
            std::tm *time_info = std::gmtime(&time); // learn gmtime
            std::uint32_t year = 1900 + time_info->tm_year; // year is from 1900
            std::uint32_t month = time_info->tm_mon;
            return (year * 100) + (month + 1); // yyyymm 
        }

        void sort_timestamps(std::vector<index_node>& timestamps){
            sort::intro_sort(
                timestamps.data(),
                timestamps.size(),
                [](const index_node& a, const index_node& b){
                    return a < b;
                }
            );
        }


        bool fill_archive_holes(std::vector<data::news_record>&  bucket, monthly_archive& archive, std::fstream& rw_stream){
            if(archive.holes.empty()) return true;
            std::vector<std::uint64_t> consumed_holes;
            int initial_hole_index = archive.holes.size(); // TODO remove this
            // start filling holes
            while(!bucket.empty() && !archive.holes.empty()){
                data::news_record record = bucket.back();
                std::uint64_t byte_offset = archive.holes.back();
                rw_stream.seekp(byte_offset, std::ios::beg);
                rw_stream.write(reinterpret_cast<char *>(&record), sizeof(record));
                // rollback when encountering a write error
                if(!rw_stream.good()){
                    // push back all the consumed holes
                    for(auto itr = consumed_holes.rbegin(); itr != consumed_holes.rend(); ++itr){
                        archive.holes.push_back(*itr);
                    }

                    return false;
                }
                // create index for the written record
                archive.timestamp_index.push_back(index_node(record.id, record.published_on, byte_offset));
                consumed_holes.push_back(byte_offset);
                bucket.pop_back();
                archive.holes.pop_back();
            }

            return true;
            
        }

        void create_indices(std::uint64_t initial_offset, std::vector<data::news_record>& bucket, monthly_archive& archive){
            for(int i = 0, n = bucket.size(); i < n; i++){
                std::uint64_t id = bucket[i].id;
                std::uint64_t b_offset = initial_offset + i * RECORD_SIZE;
                std::uint64_t p_date = bucket[i].published_on;
                archive.timestamp_index.push_back(index_node(id, p_date, b_offset));
            }
        }

        bool write_archive(std::vector<data::news_record>& bucket, monthly_archive& archive){
            if(bucket.empty()) return true;
            std::fstream rw_stream;
            rw_stream.open(archive.bin_path, std::ios::in | std::ios::out | std::ios::binary);
            if(!rw_stream.is_open()) return false;

            if(!fill_archive_holes(bucket, archive, rw_stream)) return false; 

            if(bucket.empty()) return true;
            
            rw_stream.seekp(0, std::ios::end);
            std::uint64_t initial_eof = rw_stream.tellp(); // get eof before bulk push

            rw_stream.write(reinterpret_cast<char *>(bucket.data()), bucket.size() * RECORD_SIZE); // bulk write
            // roll back if the chunk writing fails
            if(!rw_stream.good()){
                std::error_code ec;
                std::filesystem::resize_file(archive.bin_path, initial_eof, ec);
                return false;
            }

            create_indices(initial_eof, bucket, archive);

            return true;

        }

        
    public:
        cold_storage_manager(std::filesystem::path f_path) : archive_dir_path(f_path), RECORD_SIZE(sizeof(data::news_record)), 
        id_gen(f_path){}

        cold_storage_manager(const cold_storage_manager&) = delete;
        cold_storage_manager(cold_storage_manager&&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&&) = delete;

        std::uint64_t generate_id(){
            return id_gen.get_id();
        }

        void recycle_id(std::uint64_t id){
            id_gen.store_id(id);
        }

        monthly_archive* get_archive(std::uint32_t ym){
            auto itr = archives.find(ym);
            if(itr == archives.end()) return nullptr;
            return itr->second.get(); // returning pointer to object
        }

        bool create_archive(std::uint32_t ym){
            if(get_archive(ym)) return true; // if archive already exists

            std::filesystem::path dir_path = archive_dir_path / metadata::io_metadata::ARCHIVE_DIRECTORY_NAME;
            // try to create directory
            std::error_code e;
            metadata::io_metadata_manager::create_directory(dir_path , e); // create directory if doesnt exist
            if(e) return false;

            std::filesystem::path full_path = dir_path / (std::to_string(ym) + metadata::io_metadata::ARCHIVE_EXT);

            //create file if doesnt exist in append mode in order not to truncate the file if it exists
            std::fstream touch_file;
            touch_file.open(full_path, std::ios::app | std::ios::binary);
            if (!touch_file.good()) return false;
            //create archive and add it to archives
            archives[ym] = std::make_unique<monthly_archive>(ym, full_path);
            return true;
        }

        
        bool backup_records(const std::vector<data::news_record>& records){
            if(records.empty()) return true;
            // loop  through records and sort in the appropriate buckets of yyyymm
            std::unordered_map<std::uint32_t, std::vector<data::news_record>> buckets;
            for(const data::news_record& record : records){
                std::uint32_t ym = get_year_month(record.published_on);
                buckets[ym].push_back(record);
            }
            bool all_successful = true;
            // now access archive for each bucket
            for(auto&[ym, bucket] : buckets){

                create_archive(ym);
                monthly_archive *archive = get_archive(ym);
                if(!archive) continue;
                
                if(!write_archive(bucket, *archive)) all_successful = false;

                sort_timestamps(archive->timestamp_index);

            }

            return all_successful;

        }

        bool initialize(){
            std::filesystem::path src_dir = archive_dir_path / metadata::io_metadata::ARCHIVE_DIRECTORY_NAME;
            if(!std::filesystem::exists(src_dir)) return true; // directory doesnt exist yet

            bool all_succesfull = true;

            for(auto& entry : std::filesystem::directory_iterator(src_dir)){
                if(entry.path().extension() != metadata::io_metadata::ARCHIVE_EXT) continue; // skip file that is not an archive
                std::string stem = entry.path().stem().string(); // get filename
                if(!metadata::io_metadata_manager::is_positive_integer(stem)) continue; // check for invalid file name

                std::uint32_t ym = std::stoul(stem); // casting the stem to u-integer
                create_archive(ym); // create archive with ym
                monthly_archive *archive = get_archive(ym);
                if(!archive) continue; // move onto the next file if archive is not found

                bool archives_loaded = metadata::io_metadata_manager::load_metadata(archive_dir_path, *archive);
                // flag boot failure
                if(!archives_loaded) all_succesfull = false;

            }

            id_gen.load_id(); // load id_gen state

            return all_succesfull;

        }

        bool backup_metadata(){
            bool all_successful = true;
            for(auto&[ym, archive] : archives){
                bool metadata_saved;
                try{
                    metadata_saved = metadata::io_metadata_manager::save_metadata(archive_dir_path, *archive);
                }catch(std::exception e){
                    metadata_saved = false;
                }
                if(!metadata_saved) all_successful = false;
            }
            // save id gen state on disk
            id_gen.save_id();
            return all_successful;
        }


    };
}



