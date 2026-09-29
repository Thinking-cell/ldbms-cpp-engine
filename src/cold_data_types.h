#pragma once
#include<cstdint>
#include<filesystem>
#include<vector>


namespace engine{

    #pragma pack(push, 1)
    struct index_node{
        std::uint64_t id;
        std::uint64_t published_on;
        std::uint64_t byte_offset;

        index_node() = default;
        index_node(std::uint64_t n_id, std::uint64_t p_on, std::uint64_t b_offset) : id(n_id), published_on(p_on), byte_offset(b_offset){}
        // helper < overloaded function sort by time then id
        bool operator<(const index_node& i_node) const{
            if(published_on == i_node.published_on)
                return id < i_node.id;
            return published_on < i_node.published_on;
        }
    };
    #pragma pack(pop)

    struct monthly_archive{
        std::uint32_t year_month;
        std::filesystem::path bin_path;
        std::vector<index_node> timestamp_index;
        std::vector<std::uint64_t> holes;
        monthly_archive(std::uint32_t ym, std::filesystem::path f_path) : year_month(ym), bin_path(f_path){} 

    };
}