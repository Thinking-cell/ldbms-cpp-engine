#pragma once
#include<filesystem>
#include<string>
namespace metadata{

    struct io_metadata
    {
        static constexpr std::string INDEX_DIRECTORY_NAME = "index";
        static constexpr std::string ARCHIVE_DIRECTORY_NAME = "archives";
        static constexpr std::string RECYCLE_STACKS_DIRECTORY_NAME = "recycle";
        static constexpr std::string INDEX_EXT = ".idx";
        static constexpr std::string HOLE_EXT = ".holes";
        static constexpr std::string ARCHIVE_EXT = ".bin"; 
    };

    class io_metadata_manager{
        bool load_vector();
        bool save_vector();
    
    public:
        static void create_directory(std::filesystem::path path){
            if (!std::filesystem::exists(path)) std::filesystem::create_directories(path);
        }
    };
    
}