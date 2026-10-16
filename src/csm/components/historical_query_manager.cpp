#include "historical_query_engine.h"
#include "cold_storage_manager.h"
#include "../../core/cold_data_types.h"
#include <algorithm>
#include <fstream>
#include <utility>

namespace engine {



    historical_query_engine::historical_query_engine(cold_storage_manager& m) : mgr(m) {}

    void historical_query_engine::step_month(std::uint32_t& ym) {
        std::uint32_t year = ym / 100;
        std::uint32_t month = ym % 100;
        if (month == 12) {
            ym = (year + 1) * 100 + 1;
        } else {
            ym++;
        }    
    }

   
        
    bool historical_query_engine::time_series_search(std::uint64_t start_timespamp, std::uint64_t end_timestamp, std::vector<data::news_record>& results){
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

    bool historical_query_engine::delete_record(data::news_record dead_record){
        std::uint32_t ym = mgr.get_year_month(dead_record.published_on);
        monthly_archive *archive = mgr.get_archive(ym);
        if(!archive) return true; // no archive exists for the given record
        // create the key for the search
        // helper < op is written to do primary comparison on date then id
        // getting the right record with the correct id will automatically taken care
        // by the STL binary search
        engine::index_node target(dead_record.id, dead_record.published_on, 0);
        // get lower bound of the search
        // either the record or the lower value will be caught
        auto itr = std::lower_bound(archive->timestamp_index.begin(), archive->timestamp_index.end(), target);
        // check if STL binary search got the correct record if not then return false
        if(!(itr != archive->timestamp_index.end() && itr->id == dead_record.id && 
        itr->published_on == dead_record.published_on))
            return false;
        
        // record is found now delete it
        archive->holes.push_back(itr->byte_offset); // mark the record space on disk as available
        mgr.recycle_id(itr->id); // recycle the id of record for later use 
        archive->timestamp_index.erase(itr); // remove the index from archives timestamp_index
        
        return true;

    }

    bool historical_query_engine::update_record(data::news_record record){
        // check memory footprint of the passed record
        if(sizeof(data::news_record) != sizeof(record)) return false;
        std::uint32_t ym = mgr.get_year_month(record.published_on);
        monthly_archive *archive = mgr.get_archive(ym);
        if(!archive) return false; // no archive exists for the given record
        // create the key for the search
        // helper < op is written to do primary comparison on date then id
        // getting the right record with the correct id will automatically taken care
        // by the STL binary search
        engine::index_node updated_index(record.id, record.published_on, 0);
        // get the index of the record
        auto itr = std::lower_bound(archive->timestamp_index.begin(), archive->timestamp_index.end(), updated_index);
        // check if STL binary search got the correct record if not then return false
        if(!(itr != archive->timestamp_index.end() && itr->id == record.id && 
        itr->published_on == record.published_on))
            return false;
        
        // record is found now update it
        {
            std::fstream rw_stream(archive->bin_path, std::ios::binary | std::ios::in | std::ios::out);
            if(!rw_stream.is_open()) return false;

            rw_stream.seekp(itr->byte_offset, std::ios::beg); // set the write head

            rw_stream.write(reinterpret_cast<char *>(&record), sizeof(record)); // overwrite the record

            if(!rw_stream.good()) return false; // false on write error
        }

        return true;

    }

}