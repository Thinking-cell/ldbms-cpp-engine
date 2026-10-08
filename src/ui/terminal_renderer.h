#pragma once
#include <iostream>
#include <string>
#include <format>

namespace ui {
    struct ui_metadata {
        static constexpr std::uint32_t APP_WIDTH = 90;
    };

    class renderer {
    public:
        // AI assisted clear screen
        static inline void clear_screen() {
            std::cout << "\x1B[2J\x1B[3J\x1B[H" << std::flush;
        }

        static inline void render_thick_bar() {
            std::cout << std::string(ui_metadata::APP_WIDTH, '=') << "\n";
        }

        static inline void render_thin_bar() {
            std::cout << std::string(ui_metadata::APP_WIDTH, '-') << "\n";
        }

        static inline void center_text(const std::string& text) {
            std::cout << std::format("{:^{}}\n", text, ui_metadata::APP_WIDTH);
        }

        static inline void render_header(const std::string& heading) {
            render_thick_bar();
            center_text(heading);
            render_thick_bar();
        }

        static inline void wait_for_user() {
            std::string temp;
            std::cout  << "\nPress Enter to continue...\n";
            std::cin.clear(); // reset cin
            std::getline(std::cin, temp);
        }
        
        // TODO : build the custom wrapped string module and return string instead
        static inline std::string get_wrapped_string(const std::string_view text, std::string padding_ref = "    ", 
            int width = ui_metadata::APP_WIDTH - 20, int extra_padding = 8){
            if(text.empty())
                return "No content to show.";

            
            std::string padding(padding_ref.size() + extra_padding + 1, ' ');

            int char_count = 1;
            std::string result = "";
            result.reserve(text.size() + (text.size() % width) * padding.size() + 5);
            bool ident_it = false;
            
            for(int i = 0, n = text.size();i < n; i++){
                if(char_count % (width) == 0) ident_it = true;
                
                if((text[i] == ' ' || text[i] == '-') && ident_it){
                    result += '\n';
                    result += padding;
                    ident_it = false;
                }
                result += text[i];
                char_count++;
            }

            return result;

        }

        // REMOVE THIS
        // Restricts text width for terminal summaries and links | AI assisted
        static inline void print_wrapped(std::string_view text, size_t max_width, size_t indent) {
            size_t current_pos = 0;
            std::string padding(indent, ' ');
        
            while (current_pos < text.length()) {
                size_t characters_to_take = std::min(max_width, text.length() - current_pos);
                size_t break_pos = characters_to_take;

                if (current_pos + characters_to_take < text.length()) {
                    size_t last_space = text.substr(current_pos, characters_to_take).find_last_of(" \t");
                    if (last_space != std::string_view::npos) break_pos = last_space;
                }

                std::cout << padding << text.substr(current_pos, break_pos) << '\n';
                current_pos += break_pos;
                if (current_pos < text.length() && text[current_pos] == ' ') current_pos++;
            }
        }
        
    };
}