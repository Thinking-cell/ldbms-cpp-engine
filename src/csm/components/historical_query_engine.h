#pragma once
#include <cstdint>
#include <vector>

#include "../../core/news_record.h"

namespace engine {
    class cold_storage_manager; // Forward declaration

    struct search_meta_data {
        static constexpr std::uint64_t ONE_YEAR_TIMESTAMP = 60 * 60 * 24 * 365; 
    };

    class historical_query_engine {
        cold_storage_manager& mgr;

        void step_month(std::uint32_t& ym);

    public:
        historical_query_engine(cold_storage_manager& m);
        
        bool time_series_search(std::uint64_t start_timespamp, std::uint64_t end_timestamp, std::vector<data::news_record>& results);
        bool delete_record(data::news_record dead_record);
        bool update_record(data::news_record record);
    };
}