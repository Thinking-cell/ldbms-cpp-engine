#include<iostream>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include<string>

#include "ui/terminal_app.h"
#include "csm/record_manager.h"

engine::record_manager *global_db_ptr = nullptr;
// to shutdown the db in case of out of the app termination
// AI assisted
void unexpected_shutdown(int signum){
    if (global_db_ptr) {
        std::cout << "\n\n[OS KILL SIGNAL DETECTED]\n";
        std::cout << "Shutting down...\n";
        global_db_ptr->shutdown();
        std::cout << "Backup complete. Safe to exit.\n";
    }
    std::exit(signum);
}

int main(){
    std::filesystem::path db_path = "F:\\projects\\oop-python\\archival_storage";
    // create db path if doesnt exist
    if(!std::filesystem::exists(db_path)) std::filesystem::create_directories(db_path);

    engine::record_manager api(db_path);

    global_db_ptr = &api;

    // (AI)Register the OS Signal Catchers
    std::signal(SIGINT, unexpected_shutdown);  // Catches Ctrl+C
    std::signal(SIGTERM, unexpected_shutdown);

    // initialize terminal
    ui::terminal_app app(api);
    // start the app
    app.run();

    return 0;

}