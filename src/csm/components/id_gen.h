#pragma once
#include <cstdint>
#include <vector>
#include <filesystem>

namespace metadata {
    class id_gen_mgr {
        std::vector<std::uint64_t> current_id;
        std::vector<std::uint64_t> vacant_id;
        std::filesystem::path dir_path;

        void reset_current_id();

    public:
        id_gen_mgr(std::filesystem::path d_path);
        
        void load_id();
        bool save_id();
        std::uint64_t get_id();
        std::uint64_t get_total_used_ids();
        void store_id(std::uint64_t id);
    };    
}