#pragma once
#include<vector>
#include<filesystem>

#include "components/cold_storage_manager.h"
#include "components/historical_query_engine.h"
#include "components/injector.h"
#include "../core/news_record.h"
#include "../sorting/introsort.h"

namespace engine{
    class record_manager{
        cold_storage_manager cold_db;
        historical_query_engine query_engine;
        injector::record_injector cold_injector;
    
    public:
        record_manager(std::filesystem::path archive_path, std::filesystem::path injector_src) : 
        cold_db(archive_path), query_engine(cold_db), cold_injector(injector_src){}
        // for no injector instatiation
        record_manager(std::filesystem::path archive_path) : 
        cold_db(archive_path), query_engine(cold_db){}

        // DB management methods
        bool backup_records(std::vector<data::news_record> pay_load){
            return cold_db.backup_records(pay_load);
        }

        bool backup_metadata(){
            return cold_db.backup_metadata();
        }

        bool shutdown(){
            bool db_close = backup_metadata();
            return db_close;
        }

        bool boot_up(){
            return cold_db.initialize();
        }

        // search utility methods
        void sort_records(sort_order s, std::vector<data::news_record>& records){
            // return the filtered result if no sort preference is selected
            if(s == sort_order::UNSORTED) return;
            // sort if sort preference is selected
            switch (s)
            {
            case sort_order::LATEST:
                sort::intro_sort(records.data(), 
                    records.size(),
                [](const data::news_record& a, const data::news_record& b){
                    return a.published_on < b.published_on;
                });
                break;
            
            case sort_order::OLDEST:
                sort::intro_sort(records.data(), 
                    records.size(),
                [](const data::news_record& a, const data::news_record& b){
                    return b.published_on < a.published_on;
                });
                break;
            
            case sort_order::MOST_VIEWS:
                sort::intro_sort(records.data(), 
                    records.size(),
                [](const data::news_record& a, const data::news_record& b){
                    return a.views < b.views;
                });
                break;

            default:
                break;
            }
            
        }

        std::vector<data::news_record> search_records(const query_request& query){
            std::vector<data::news_record> raw_result;
            // fetch all the records within the time range
            if(!query_engine.time_series_search(query.start_time, query.end_time, raw_result))
                return {};
            
            // to save the actual processed results
            std::vector<data::news_record> filtered_result;

            // check if a keyword was inputted
            // skip the loop if there is nothing to search
            if(query.keyword.empty()){
                filtered_result = std::move(raw_result); // make move all records to filtered result
                sort_records(query.sort_by, filtered_result);
                return filtered_result;
            }
            
            // now filter each record
            for(auto& record : raw_result){

                bool matched = false;

                // check if the keyword has a match in title
                if(query.search_title){
                    std::string_view title_view(record.title.data, record.title.length);
                    if(title_view.find(query.keyword) != std::string_view::npos) matched = true;
                }
                if(!matched && query.search_summary){
                    std::string_view summary_view(record.summary.data, record.summary.length);
                    if(summary_view.find(query.keyword) != std::string_view::npos) matched = true;
                }
                if(!matched && query.search_author){
                    std::string_view author_view(record.author.data, record.author.length);
                    if(author_view.find(query.keyword) != std::string_view::npos) matched = true;
                }

                if(!matched) continue;
                // move the record in the filtered result
                filtered_result.push_back(std::move(record));
                
            }

            
            sort_records(query.sort_by, filtered_result);
            return filtered_result;

        }

        

        bool insert_records(data::news_record record){
            std::vector<data::news_record> pay_load;
            pay_load.push_back(record);
            return cold_db.backup_records(pay_load);
        }
        bool insert_records(std::vector<data::news_record> records){
            return cold_db.backup_records(records);
        }

        bool delete_records(data::news_record dead_rec){
            return query_engine.delete_record(dead_rec);
        }
        bool delete_records(std::vector<data::news_record> dead_recs){
            bool all_successful = true;
            for(auto& d_record : dead_recs){
                if(!query_engine.delete_record(d_record)) all_successful = false;
            }
            return all_successful;
        }

        bool update_record(data::news_record record){
            return query_engine.update_record(record);
        }

        // injector utility methods
        bool inject_records(){
            if(!cold_injector.is_path_valid()) return false;
            // creating the wrapper lambda in order to pass the backup function to the injector
            auto vein = [this](const std::vector<data::news_record>& records){ return this->cold_db.backup_records(records);};
            // creating the wrapper lambda to pass the id generation function to the injector
            auto id_gen = [this](){return this->cold_db.generate_id();};

            bool records_injected = cold_injector.inject_records(vein, id_gen);
            return records_injected;
        }

        bool prime_injector(std::filesystem::path injector_src){
            return cold_injector.set_path(injector_src);
        }


    };
}