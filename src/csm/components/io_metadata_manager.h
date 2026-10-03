#pragma once
#include<filesystem>
#include<fstream>
#include<string>
#include<stdexcept>
#include<vector>

#include "../../core/news_record.h"
#include "../../core/cold_data_types.h"
#include "../../core/cold_data_types.h"

namespace metadata{

    struct io_metadata
    {
        static constexpr std::string INDEX_DIRECTORY_NAME = "index";
        static constexpr std::string ARCHIVE_DIRECTORY_NAME = "archives";
        static constexpr std::string RECYCLE_STACKS_DIRECTORY_NAME = "recycle";
        static constexpr std::string ID_DIRECTORY_NAME = "id";
        static constexpr std::string ID_GEN_STEM = "id_gen";
        static constexpr std::string VACANT_ID_STEM = "vacant_id";
        static constexpr std::string INDEX_EXT = ".idx";
        static constexpr std::string HOLE_EXT = ".holes";
        static constexpr std::string ARCHIVE_EXT = ".bin"; 
        static constexpr std::string ID_EXT = ".ii";
    };


    class io_metadata_manager{

        friend class id_gen_mgr;
        // RAII rollback class for disk write
        class transaction{
            std::filesystem::path tmp_path;
            std::filesystem::path full_path;
            std::ofstream w_stream;
            bool is_committed;

        public:
            transaction(std::filesystem::path f_path) : full_path(f_path), tmp_path(f_path.string() + ".tmp"), is_committed(false){
                w_stream.open(tmp_path, std::ios::out | std::ios::binary | std::ios::trunc);
                if(!w_stream.good())
                    throw std::runtime_error("Unable to open metadata write stream for:\n" + full_path.string());
            }
            ~transaction(){
                if(w_stream.is_open()) w_stream.close();
                if(!is_committed){
                    std::error_code ec;
                    // removing the tmp file in case of transaction failure or exception thrown
                    std::filesystem::remove(tmp_path, ec);
                }
            }

            template<typename T>
            bool write_vector(const std::vector<T>& pay_load){
                if(!w_stream.is_open()) return false;
                w_stream.write(reinterpret_cast<const char*>(pay_load.data()), pay_load.size() * sizeof(T));
                if (!w_stream.good()) 
                    throw std::runtime_error("Disk write failed (out of space or hardware error).");
                
                return true;
                
            }

            bool commit(){
                if(w_stream.is_open()) w_stream.close();

                // renaming the tmp file to the original file name
                std::error_code ec;
             
                std::filesystem::rename(tmp_path, full_path, ec);
                
                if(ec) return false; 
                
                is_committed = true;
                return is_committed;
            }

        };

        template<typename T>
        static bool load_vector(const std::filesystem::path base_dir, const std::filesystem::path sub_dir,
        const std::string file_name, std::vector<T>& target){
            std::filesystem::path full_path = base_dir / sub_dir / file_name;
            // if no file exists then return true
            if(!std::filesystem::exists(full_path)){
                target.clear();
                return true;
            }

            std::ifstream r_stream;
            // open and put the read head at eof
            r_stream.open(full_path, std::ios::in | std::ios::ate | std::ios::binary);
            std::streamsize file_size = r_stream.tellg(); // get the size of the data inside
            r_stream.seekg(0, std::ios::beg); // reset the read head
            // return true if file exists but is empty
            if(file_size == 0){
                target.clear();
                return true;
            }
            // return false if file's memory footprint is mismatched with the data type
            if(file_size % sizeof(T) != 0){
                target.clear();
                return false;
            }
            // expand the vector and set current size to accomadate the incoming pay load
            target.resize(file_size / sizeof(T)); 
            r_stream.read(reinterpret_cast<char *>(target.data()), file_size);
            // return false in case of a read failure
            if(!r_stream.good()){
                target.clear(); // clear the vector
                return false;
            }

            return true;

        }

        template<typename T>
        static bool save_vector(const std::filesystem::path base_dir, const std::filesystem::path sub_dir,
        const std::string file_name, const std::vector<T>& pay_load){
            std::filesystem::path dir_path = base_dir / sub_dir;
            std::filesystem::path full_path = dir_path / file_name;
            std::error_code e;
            create_directory(dir_path, e);
            if(e) return false;
            // errors thrown by transaction will handled by upper layer
            transaction tx(full_path);
            if(!tx.write_vector(pay_load)) return false;
            if(!tx.commit()) return false;

            return true;
        }
    
    public:
        static void create_directory(std::filesystem::path path, std::error_code& e){
            if (!std::filesystem::exists(path)) std::filesystem::create_directories(path, e);
        }
        static bool is_positive_integer(const std::string& s){
            if (s.empty()) return false;
            for(const auto& c : s){
                if(!std::isdigit(c)) return false;
            }
            return true;
        }

        static bool load_metadata(const std::filesystem::path base_dir, engine::monthly_archive& archive){
            std::filesystem::path holes_sub_dir = base_dir / io_metadata::RECYCLE_STACKS_DIRECTORY_NAME;
            std::filesystem::path timestamps_sub_dir = base_dir / io_metadata::INDEX_DIRECTORY_NAME;
            std::string holes_filename = std::to_string(archive.year_month) + io_metadata::HOLE_EXT;
            std::string timestamps_filename = std::to_string(archive.year_month) + io_metadata::INDEX_EXT;

            bool holes_loaded = load_vector(base_dir, holes_sub_dir, holes_filename, archive.holes);
            bool index_loaded = load_vector(base_dir, timestamps_sub_dir, timestamps_filename, archive.timestamp_index);

            return holes_loaded && index_loaded;
        }

        static bool save_metadata(const std::filesystem::path base_dir, const engine::monthly_archive& archive){
            std::filesystem::path holes_sub_dir = base_dir / io_metadata::RECYCLE_STACKS_DIRECTORY_NAME;
            std::filesystem::path timestamps_sub_dir = base_dir / io_metadata::INDEX_DIRECTORY_NAME;
            std::string holes_filename = std::to_string(archive.year_month) + io_metadata::HOLE_EXT;
            std::string timestamps_filename = std::to_string(archive.year_month) + io_metadata::INDEX_EXT;

            bool holes_saved = save_vector(base_dir, holes_sub_dir, holes_filename, archive.holes);
            bool index_saved = save_vector(base_dir, timestamps_sub_dir, timestamps_filename, archive.timestamp_index);

            return holes_saved && index_saved;
        }
    };

   
    
}