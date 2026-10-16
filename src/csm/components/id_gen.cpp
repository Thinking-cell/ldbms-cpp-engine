#include "id_gen.h"
#include "io_metadata_manager.h"

namespace metadata {

    void id_gen_mgr::reset_current_id() {
        current_id.clear();
        current_id.push_back(0);
    }

    id_gen_mgr::id_gen_mgr(std::filesystem::path d_path) : dir_path(d_path){
        current_id.push_back(0);
    }
    
    void id_gen_mgr::load_id(){
        // loading the saved id count
        if(!io_metadata_manager::load_vector(dir_path, io_metadata::ID_DIRECTORY_NAME, 
        (io_metadata::ID_GEN_STEM + io_metadata::ID_EXT), current_id))
            reset_current_id();
        
        // TODO: remove this, Redundant
        // if the file was empty or didnt exist yet, the load functions clears the vector
        if(current_id.empty()) reset_current_id();
        
        // load vacant id vector
        if(!io_metadata_manager::load_vector(dir_path, io_metadata::ID_DIRECTORY_NAME,
        (io_metadata::VACANT_ID_STEM + io_metadata::ID_EXT), vacant_id))
            vacant_id.clear();
    }

    bool id_gen_mgr::save_id(){
        bool id_saved = io_metadata_manager::save_vector(dir_path, io_metadata::ID_DIRECTORY_NAME, 
        (io_metadata::ID_GEN_STEM + io_metadata::ID_EXT), current_id);

        bool vacant_saved = io_metadata_manager::save_vector(dir_path, io_metadata::ID_DIRECTORY_NAME,
        (io_metadata::VACANT_ID_STEM + io_metadata::ID_EXT), vacant_id);

        return id_saved && vacant_saved;
    }
    

    std::uint64_t id_gen_mgr::get_id(){
        if(!vacant_id.empty()){
            std::uint64_t id = vacant_id.back();
            vacant_id.pop_back();
            return id;
        }

        return current_id[0]++;
    }

    std::uint64_t id_gen_mgr::get_total_used_ids(){
        // this is a shenanigan implementation, dont question it
        if (current_id[0] < vacant_id.size()) {
            return vacant_id.size();
        }
        return current_id[0] - vacant_id.size();
    }

    void id_gen_mgr::store_id(std::uint64_t id){
        vacant_id.push_back(id);    
    }

    

}