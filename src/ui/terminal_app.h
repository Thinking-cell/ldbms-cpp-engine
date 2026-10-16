#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../csm/record_manager.h"
#include "terminal_utils.h"
#include "terminal_renderer.h"
#include "../core/td_array.h"

namespace ui {

    class terminal_app {
    
        engine::record_manager& api;
        data::td_array<data::news_record> active_results;
        std::uint32_t current_page_idx = 0;
        std::uint32_t PAGE_LIMIT = 10;
        std::filesystem::path latest_bin = "F:\\projects\\oop-python\\injector_src\\latest\\packed_news.bin";

        // Internal Helpers
        std::uint32_t get_current_page();
        std::uint32_t get_total_pages();
        std::uint64_t input_unix_date(std::string heading, std::string user_context);
        data::td_array<data::news_record> fetch_latest(int limit = 100);
        data::td_array<data::news_record> fetch_latest_from_db(int limit = 100);

        // custom min tempalate for primitive mix type comparisons
        template<typename T, typename S>
        auto get_min(T a, S b) {
            return (a < b) ? a : b;
        }

        // UI Controllers
        void render_page();
        void prompt_new_query();
        void prompt_categorize();
        void prompt_delete();
        void prompt_update();
        void prompt_insert();
        void prompt_injection();

    public:
        terminal_app(engine::record_manager& mgr);
        void run();
    };

}