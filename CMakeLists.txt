cmake_minimum_required(VERSION 3.20)

# 1. Name the project
project(NewsRecordManager LANGUAGES CXX)

# 2. Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. Specify all source (.cpp) files
set(SOURCES
    src/main.cpp
    src/ui/terminal_app.cpp
    src/csm/record_manager.cpp
    src/csm/components/id_gen.cpp
    src/csm/components/cold_storage_manger.cpp
    src/csm/components/historical_query_manager.cpp
)

# 4. Define the executable target
add_executable(main ${SOURCES})

# 5. Tell the compiler to use "src" as the root include directory
# This allows clean includes like #include "core/news_record.h" anywhere in the project!
target_include_directories(main PRIVATE src)

# 6. Enable standard compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra)
endif()