#pragma once
#include<algorithm>
#include<cstdint>
#include<fstream>
#include<vector>
#include<unordered_map>

#include "cold_storage_manager.h"
#include "news_record.h"

namespace engine{

    struct search_meta_data{
        static constexpr std::uint64_t ONE_YEAR_TIMESTAMP = 60 * 60 * 24 * 365; 
    };

    class historical_search_engine{

        engine::cold_storage_manager& mgr;
        void step_month(std::uint32_t& ym){
            std::uint32_t year = ym / 100;
            std::uint32_t month = ym % 100;
            if (month == 12) {
                ym = (year + 1) * 100 + 1; // 202412 becomes 202501
            } else {
                ym++;
            }    
        }

    public:
        historical_search_engine(engine::cold_storage_manager& m) : mgr(m) {}
        
        bool time_series_search(std::uint64_t start_timespamp, std::uint64_t end_timestamp, std::vector<data::news_record>& results){
            if(start_timespamp > end_timestamp ||
                 (end_timestamp - start_timespamp) > search_meta_data::ONE_YEAR_TIMESTAMP) return false;
                
            std::uint32_t start_ym = mgr.get_year_month(start_timespamp);
            std::uint32_t end_ym = mgr.get_year_month(end_timestamp);
            // retireving all the available archives
            std::vector<monthly_archive*> searchable_archives;
            for(std::uint32_t current_ym = start_ym, end = end_ym; current_ym <= end; step_month(current_ym)){
                monthly_archive* archive = mgr.get_archive(current_ym);
                if(!archive) continue;
                searchable_archives.push_back(archive);
            }

            if(searchable_archives.empty()) return true;

            for(auto& archive_ptr : searchable_archives){
                // get the entire archive
                auto start_itr = archive_ptr->timestamp_index.begin();
                auto end_itr = archive_ptr->timestamp_index.end();

                // in case of the starting archive only extract records > start_timestamp
                if(archive_ptr->year_month == start_ym){
                    // search for the valid ym records in the archive
                    start_itr = std::lower_bound(
                        archive_ptr->timestamp_index.begin(), // Lower bound
                        archive_ptr->timestamp_index.end(), // upper bound
                        start_timespamp,
                        [](const index_node& n, const std::uint64_t key){
                            return n.published_on < key;
                        }
                    );
                }

                // in case the end archive, only extract records < end_timestamp
                if(archive_ptr->year_month == end_ym){
                    // search for the valid ym records in the archive
                    end_itr = std::upper_bound(
                        start_itr, // in case of one file search start from start itr
                        archive_ptr->timestamp_index.end(),
                        end_timestamp,
                        [](const std::uint64_t key, const index_node& n){
                            return key < n.published_on;
                        }
                    );
                }


                if (start_itr == end_itr) continue; // no records in this time window

                // open write stream for reading the records
                std::ifstream r_stream(archive_ptr->bin_path, std::ios::binary);
                if (!r_stream.is_open()) continue;
                // looping the records
                for(auto& itr = start_itr; itr != end_itr; ++itr){
                    r_stream.seekg(itr->byte_offset, std::ios::beg);
                    data::news_record new_record;
                    r_stream.read(reinterpret_cast<char *>(&new_record), sizeof(new_record));
                    if(!r_stream.good()) continue; // skip in case of read failure
                    results.push_back(std::move(new_record)); // move the record in the result vector
                }
            }

            return true;
            
        }
    };
}