#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <format>
#include <iostream>
#include <unordered_set>

#include "terminal_app.h"

namespace ui{
    // constructor
    terminal_app::terminal_app(engine::record_manager& mgr) : api(mgr) {}

    std::uint32_t terminal_app::get_current_page(){
            return (current_page_idx / PAGE_LIMIT) + 1;
        }

    std::uint32_t terminal_app::get_total_pages(){
        return active_results.empty() ? 1 : ((active_results.size() - 1) / PAGE_LIMIT) + 1;
    }

    std::uint64_t terminal_app::input_unix_date(std::string heading, std::string user_context){
        // get unix timestamp from user 
        do
        {
            std::string input;
            std::cout << "\n" << user_context;
            std::getline(std::cin, input); // get input
            parser::trim_spaces(input); // remove all spaces for better sanitation
            if(input.empty()) return 0; // return if the user entered nothing
            std::uint64_t return_value = parser::parse_date_to_timestamp(input);
            if(return_value != 0)
                return return_value;

            renderer::clear_screen();
            renderer::render_header(heading);
            std::cout << "\nInvalid Date, Please Try Again->\n";

        } while (true);

        return 0;
    }

    // fetches the latest of last 12 year records
    std::vector<data::news_record> terminal_app::fetch_latest(int limit){
        std::vector<data::news_record> results;
        std::uint64_t end_time = std::time(nullptr);
        std::uint64_t start_time = end_time - engine::search_meta_data::ONE_YEAR_TIMESTAMP;
        engine::query_request query;
        query.start_time = start_time;
        query.end_time = end_time;
        query.keyword = "";
        query.search_title = true;
        query.sort_by = engine::sort_order::LATEST;

        // search for last 12 years of records
        for(int i = 0; i < 12; i++){
            results = api.search_records(query);
            if(!results.empty()){
                // limit records to 100
                if(results.size() > static_cast<size_t>(limit)) {
                    results.resize(limit); 
                }
                return results;
            }else{
                // move one year back if no results were found
                query.start_time -= engine::search_meta_data::ONE_YEAR_TIMESTAMP;
                query.end_time -= engine::search_meta_data::ONE_YEAR_TIMESTAMP;
            }
        }
        results.clear();
        return results;


    }


    void terminal_app::render_page(){
        renderer::clear_screen();
        std::cout << "\n";
        renderer::render_thick_bar();
        // calculate total pages to be displayed
        std::uint32_t total_pages = get_total_pages();
        // displaying using string formatting
        std::cout << std::format(" PAGE {} of {} | TOTAL RECORDS: {}\n",
            get_current_page(), total_pages, active_results.size());

        renderer::render_thick_bar();

        if(active_results.empty()){
            renderer::center_text("No records to show!");
            renderer::render_thick_bar();
            return;
        }

        std::uint32_t end_index = get_min((current_page_idx + PAGE_LIMIT), active_results.size());

        // looping the active result
        for(std::uint32_t i = current_page_idx; i < end_index; i++){
            const data::news_record& record = active_results[i];
            // get all the text using string view to avoid copying of the text
            std::string_view title_view(record.title.data, record.title.length);
            std::string_view summary_view(record.summary.data, record.summary.length);
            std::string_view link_view(record.link.data, record.link.length);
            // converting unix-timestamp to readable yyyy-mm-dd format, AI assisted
            std::time_t p_time = record.published_on;
            char date_buf[20];
            std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", std::gmtime(&p_time));
            // formatting the row
            std::string row = std::format(
                "[{:<16}]       DATE: {:<12}     VIEWS: {:<16}\n\n"
                "   > Title:    {}\n"
                "   > Summary:  {}\n"
                "   > Link:     {}\n",
                record.id, date_buf, record.views, title_view, summary_view, link_view
            );
            // printing the row
            std::cout << row;
            if(i < end_index - 1)
                renderer::render_thin_bar();

        }

        renderer::render_thick_bar();
    }

    void terminal_app::prompt_new_query(){
        renderer::clear_screen();
        std::string heading = "New Query";
        //page intro
        renderer::render_header(heading);

        engine::query_request query;
        // get start time from user
        query.start_time = input_unix_date(heading, "\nEnter Start Date (YYYY-MM-DD): ");
        if(query.start_time == 0){
            std::cout << "\nDefaulting to 1 year ago.\n";
            // set start time one year ago from present
            query.start_time = std::time(nullptr) - (engine::search_meta_data::ONE_YEAR_TIMESTAMP);
        }

        // get end time from user
        do{
            query.end_time = input_unix_date(heading, "Enter End Date (YYYY-MM-DD): ");
            if(query.end_time == 0){
                std::cout << "\nDefaulting to Present.\n";
                query.end_time = std::time(nullptr);
            }
            if(query.start_time > query.end_time){
                renderer::clear_screen();
                renderer::render_header(heading);
                std::cout << "\nEnd date must come before Start date!\n";
            }
        }while(query.start_time > query.end_time);

        // get keyword from user
        std::cout << "Search Keyword (Leave blank for all): ";
        std::getline(std::cin, query.keyword); // using getline to take multiple words as input

        std::string input;
        if(!query.keyword.empty()){
            std::cout << "Search in [1]Title [2]Summary [3]Author (e.g., 1,3): ";
            std::getline(std::cin, input);
            // storing parsed anser in a set
            auto choices = parser::parse_category_set(input); 
            // checking if the set contains the value
            query.search_title = choices.contains(1);
            query.search_summary = choices.contains(2);
            query.search_author = choices.contains(3);
            // if user chose none then search by title as default
            if(!query.search_title && !query.search_summary && !query.search_author)
                query.search_title = true;

        }

        // get sort order
        input.clear();
        std::cout << "Sort: [0]Unsorted [1]Latest [2]Oldest [3]Most Views: ";
        std::getline(std::cin, input);
        // check for valid order
        try { query.sort_by = static_cast<engine::sort_order>(std::stoi(input)); } 
        catch(...) { query.sort_by = engine::sort_order::LATEST; }

        std::cout << "\nSearching...\n";
        active_results = api.search_records(query);
        current_page_idx = 0; // reset the current page index
        
    }

    void terminal_app::prompt_categorize(){
        if(active_results.empty()){
            renderer::clear_screen();
            renderer::render_header("No results available to categorize!");
            return;
        }

        renderer::clear_screen();
        renderer::render_header("Categorize Results");
        // store the categories and the nations
        std::unordered_set<std::uint8_t> categories;
        std::unordered_set<std::string> nations;
        for(const auto& record : active_results){
            categories.insert(record.category_id);
            nations.insert(std::string(record.country_code));
        }
        // display available categories and nations
        std::cout << "\nAvailable Categories in view: ";
        for (auto c : categories) std::cout << (int)c << " ";
        std::cout << "\nAvailable Nations in view: ";
        for (auto& n : nations) std::cout << n << " ";
        std::cout << '\n';
        // get input from the user
        std::string cat_input, nat_input;
        std::cout << "\n\nEnter target categories (comma-separated, leave blank for all): ";
        std::getline(std::cin, cat_input);
        std::cout << "Enter target nations (comma-separated, leave blank for all): ";
        std::getline(std::cin, nat_input);
        // removing whitespaces from the inputs
        parser::trim_spaces(cat_input);
        parser::trim_spaces(nat_input);
        // parsing user input, only valid options will be used
        // invalid category, nations will be not included
        auto target_cats = parser::parse_category_set(cat_input);
        auto target_nats = parser::parse_nation_set(nat_input);
        // calling api categorize method
        api.categorize_records(target_cats, target_nats, active_results);
        current_page_idx = 0;
        std::cout << "Results Filtered->\n";

    }

    void terminal_app::prompt_delete(){
        renderer::clear_screen();
        std::cout << "\n\n";
        render_page();
        renderer::render_header("Delete Mode");
        std::string input;
        std::cout << "\nEnter Record IDs to delete(comma-separated): ";
        std::getline(std::cin, input);
        parser::trim_spaces(input);
        // parse the input, parser will ignore invalid inputs
        std::vector<std::uint64_t> target_ids = parser::parse_id_list(input);
        if(target_ids.empty()){
            renderer::clear_screen();
            renderer::render_header("No matching IDs found in current results!");
            return;
        }

        // get the records to be deleted
        std::vector<data::news_record> dead_records;
        for(auto& record: active_results){
            if(std::find(target_ids.begin(), target_ids.end(), record.id) != target_ids.end()){
                dead_records.push_back(record);
            }
        }

        // check if record extraction succeeded
        if (dead_records.empty()) {
            renderer::clear_screen();
            renderer::render_header("No matching records found in current results!");
            return;
        }

        // try to delete the records
        if(api.delete_records(dead_records)){
            // delete records from the active results if deletion succeeded
            std::erase_if(active_results, [&target_ids](const data::news_record& record){
                return std::find(target_ids.begin(), target_ids.end(), record.id) != target_ids.end();
            });
            std::cout << dead_records.size() << " records deleted permanently.\n";
            api.backup_metadata();
        }else{
            renderer::clear_screen();
            renderer::render_header("Deletion Failed. Database error!");
        }

    }

    void terminal_app::prompt_update(){
        renderer::clear_screen();
        
        render_page();
        renderer::render_header("Update Mode");

        std::cout << "\nEnter single Record ID to update: ";
        std::string input;
        std::getline(std::cin, input);
        parser::trim_spaces(input);

        std::vector<std::uint64_t> target_ids = parser::parse_id_list(input);
        if(target_ids.size() != 1){
            std::cout << "Please enter exactly one valid ID.\n";
            return;
        }

        data::news_record* target_ptr = nullptr;
        for(auto& record : active_results){
            if(record.id == target_ids[0]){
                target_ptr = &record;
            }
        }

        if (!target_ptr) {
            std::cout << "Record not found in current results.\n";
            return;
        }

        // TODO: interactive update terminal
        bool editing = true;
        while(editing){
            renderer::clear_screen();
            renderer::render_header("Editing Record ID: " + std::to_string(target_ptr->id));

            std::string_view title_view(target_ptr->title.data, target_ptr->title.length);
            std::cout << "\n [1] Views:    " << target_ptr->views
                        << "\n [2] Category: " << (int)target_ptr->category_id
                        << "\n [3] Nation:   " << target_ptr->country_code
                        << "\n [4] Title:    " << title_view
                        << "\n\n [S] Save & Exit"
                        << "\n [C] Cancel & Discard"
                        << "\n\nSelect field to edit> ";


            input.clear();
            std::getline(std::cin, input);
            parser::trim_spaces(input);
            char choice = toupper(input[0]);

            switch (choice)
            {
            case '1':
                input.clear();
                std::cout << "New Views: ";
                std::getline(std::cin, input);
                parser::trim_spaces(input);
                try { 
                    target_ptr->views = std::stoul(input); 
                } catch(...) {
                    std::cout << "Invalid input!\n";
                }
                break;

            case '2':
                input.clear();
                std::cout << "New Category (0-255): ";
                std::getline(std::cin, input);
                parser::trim_spaces(input);
                try { 
                    target_ptr->category_id = static_cast<std::uint8_t>(std::stoi(input)); 
                } catch(...) {
                    std::cout << "Invalid input!\n";
                }
                break;

            case '3':
                input.clear();
                std::cout << "New Nation (Max 3 chars): ";
                std::getline(std::cin, input); 
                parser::trim_spaces(input);
                // get only the first 3 characters of the input
                for(int i = 0; i < 3 && i < input.length(); ++i) target_ptr->country_code[i] = toupper(input[i]);
                target_ptr->country_code[get_min(input.length(), 3)] = '\0';
                break;

            case '4':
                input.clear();
                std::cout << "New Title: ";
                std::getline(std::cin, input);
                target_ptr->title = data::fixed_string<256>(input.c_str());
                break;
            
            case 'S':
                if (api.update_record(*target_ptr)) std::cout << "\nRecord Updated.\n";
                else std::cout << "\nUpdate failed.\n";
                editing = false;
                break;
            
            case 'C':
                std::cout << "\nUpdate discarded.\n";
                renderer::wait_for_user();
                editing = false;
                break;
            
            default:
                break;
            }
        }

    }

    void terminal_app::prompt_insert(){
        renderer::clear_screen();
        renderer::render_header("Insert New Record");

        data::news_record record{};

        std::string input;

        std::cout << "\nTitle: ";
        std::getline(std::cin, input);
        if(input.empty()) input = "Default Title";
        record.title.set_data(input.c_str());

        input.clear();
        std::cout << "\nSummary: ";
        std::getline(std::cin, input);
        if(input.empty()) input = "Default Summary";
        record.summary.set_data(input.c_str());

        input.clear();
        std::cout << "\nAuthor: ";
        std::getline(std::cin, input);
        if(input.empty()) input = "John";
        record.author.set_data(input.c_str());

        input.clear();
        std::cout << "\nLink: ";
        std::getline(std::cin, input);
        if(input.empty()) input = "www.dummy.com";
        record.link.set_data(input.c_str());

        input.clear();
        std::cout << "\nNation (3 chars): ";
        std::getline(std::cin, input);
        if(input.empty()) input = "";
        for(int i = 0; i < 3 && i < input.length(); ++i) record.country_code[i] = toupper(input[i]);
        record.country_code[get_min(input.length(), 3)] = '\0';
        
        input.clear();
        std::cout << "\nCategory ID (0-255): ";
        std::getline(std::cin, input);
        try { record.category_id = static_cast<std::uint8_t>(std::stoi(input)); } 
        catch(...) { record.category_id = 0; }

        record.views = 1;
        record.published_on = std::time(nullptr);
        record.id = api.generate_id();
        if(api.insert_records(record)){
            std::cout << "\nRecord successfully inserted! ID: " << record.id << "\n";
            api.backup_metadata();
        }else{
            std::cout << "\nFailed to write record to disk.\n";
        }

    }


    void terminal_app::prompt_injection(){
        renderer::clear_screen();
        renderer::render_header("Injection Mode");
        std::string path_string = "";
        std::cout << "Enter Injection source file path: ";
        std::getline(std::cin, path_string);
        std::cout << "\nPrimining Injector...\n";
        std::filesystem::path i_path = path_string;
        if(!api.prime_injector(i_path)){
            std::cout << "\nNo valid injection source found.\n";
            return;
        }

        std::cout << "\nInjector Primed and ready to fire.\n";

        renderer::wait_for_user();

        if(api.inject_records()){
            std::cout << "Data injected successfully.\n";
            api.backup_metadata();
        }
        else
            std::cout << "Injection failed!\n"; 

    }
    void terminal_app::run(){
        renderer::clear_screen();
        render_page();

        if(!api.boot_up()){
            renderer::render_header("\nFatal Error: Failed to boot DB!\n");
            renderer::wait_for_user();
            return;
        }

        std::cout << "\nFetching latest available records...\n";
        // pull 100 latest available news
        active_results = fetch_latest(100);

        bool running = true;
        while(running){
            renderer::clear_screen();
            render_page();

            std::cout << "\n OPTIONS: [N]ext | [P]rev | [Q]uery | [C]ategorize" 
            << "\n\n         | [A]dd | [D]elete | [U]pdate | [I]nject | [E]xit\n> ";

            std::string choice_string;
            std::getline(std::cin, choice_string);
            parser::trim_spaces(choice_string);

            if(choice_string.empty()) continue;

            char choice = toupper(choice_string[0]);
                
            switch (choice)
            {
            case 'N':
                if(current_page_idx + PAGE_LIMIT < active_results.size()) current_page_idx += PAGE_LIMIT;
                break;
            
            case 'P':
                if(current_page_idx >= PAGE_LIMIT) current_page_idx -= PAGE_LIMIT;
                break;

            case 'Q':
                prompt_new_query();
                break;
            
            case 'C':
                prompt_categorize();
                renderer::wait_for_user();
                break;
            
            case 'A':
                prompt_insert();
                renderer::wait_for_user();
                break;

            case 'D':
                prompt_delete();
                renderer::wait_for_user();
                break;
            
            case 'U':
                prompt_update();
                renderer::wait_for_user();
                break;

            case 'I':
                prompt_injection();
                renderer::wait_for_user();
                break;

            case 'E':
                running = false;
                break;
            
            default:
                break;
            }

        }

        renderer::clear_screen();
        renderer::render_header("System shutting down.");
        std::cout << "\nSaving db state->closing database...\n";
        if(api.shutdown()) std::cout << "Shutdown sequence successful.";
        else std::cout << "Shutdown sequence failed!\n Previous state might be lost.";
        renderer::wait_for_user();
    }


    
}
