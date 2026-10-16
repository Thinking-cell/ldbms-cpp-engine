#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../../core/cold_data_types.h"
#include "../../core/news_record.h"
#include "id_gen.h"

namespace engine {
    class cold_storage_manager {
        const std::uint16_t RECORD_SIZE;
        std::filesystem::path archive_dir_path;
        metadata::id_gen_mgr id_gen;
        std::unordered_map<std::uint32_t, std::unique_ptr<monthly_archive>> archives;

        void sort_timestamps(std::vector<index_node>& timestamps);
        bool fill_archive_holes(std::vector<data::news_record>& bucket, monthly_archive& archive, std::fstream& rw_stream);
        void create_indices(std::uint64_t initial_offset, std::vector<data::news_record>& bucket, monthly_archive& archive);
        bool write_archive(std::vector<data::news_record>& bucket, monthly_archive& archive);

    public:
        cold_storage_manager(std::filesystem::path f_path);

        cold_storage_manager(const cold_storage_manager&) = delete;
        cold_storage_manager(cold_storage_manager&&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&) = delete;
        cold_storage_manager& operator=(cold_storage_manager&&) = delete;

        std::uint64_t generate_id();
        std::uint64_t get_total_records();
        void recycle_id(std::uint64_t id);
        std::uint32_t get_year_month(std::uint64_t timestamp);
        monthly_archive* get_archive(std::uint32_t ym);
        bool create_archive(std::uint32_t ym);
        bool backup_records(const std::vector<data::news_record>& records);
        bool initialize();
        bool backup_metadata();
    };
}