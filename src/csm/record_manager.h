#pragma once
#include <filesystem>
#include <vector>
#include <unordered_set>

#include "components/cold_storage_manager.h"
#include "components/historical_query_engine.h"
#include "components/injector.h"
#include "../core/news_record.h"
#include "../sorting/introsort.h"

namespace engine {
    class record_manager {
        cold_storage_manager cold_db;
        historical_query_engine query_engine;
        injector::record_injector cold_injector;
    
    public:
        record_manager(std::filesystem::path archive_path, std::filesystem::path injector_src);
        // for no injector instatiation
        record_manager(std::filesystem::path archive_path);
        ~record_manager();

        // DB management methods
        bool backup_records(std::vector<data::news_record> pay_load);
        bool backup_metadata();
        std::uint64_t generate_id();
        bool shutdown();
        bool boot_up();
        std::uint64_t get_total_records();

        // search utility methods
        void sort_records(sort_order s, std::vector<data::news_record>& records);
        void categorize_records(const std::unordered_set<std::uint16_t>& categories, std::vector<data::news_record>& records);
        std::vector<data::news_record> search_records(const query_request& query);

        // CUD operations
        bool insert_records(data::news_record record);
        bool insert_records(std::vector<data::news_record>& records);
        bool delete_records(data::news_record dead_rec);
        bool delete_records(std::vector<data::news_record>& dead_recs);
        bool update_record(data::news_record record);

        // injector utility methods
        bool inject_records(bool show_status = true);
        bool prime_injector(std::filesystem::path injector_src);
    };
}