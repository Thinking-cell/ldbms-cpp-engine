#include <iostream>
#include <cassert>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>

/**
 * AI genereated test script, checks
 * all the methods of the records manager
 */
// Include your facade (adjust the path to match your folder structure)
#include "csm/record_manager.h" 

// --- Test Framework Helpers ---
int tests_passed = 0;
int tests_failed = 0;

#define EXPECT_TRUE(condition, msg) \
    if (condition) { \
        std::cout << "[PASS] " << msg << "\n"; \
        tests_passed++; \
    } else { \
        std::cerr << "[FAIL] " << msg << "\n"; \
        tests_failed++; \
    }

// --- Data Generators ---
data::news_record make_record(std::uint64_t timestamp, const char* title, 
                              const char* author, const char* nation, 
                              std::uint8_t category, std::uint32_t views) {
    data::news_record rec{};
    rec.published_on = timestamp;
    rec.category_id = category;
    rec.views = views;
    
    // Safely copy country code
    std::string n(nation);
    for(size_t i = 0; i < 4 && i < n.size(); ++i) rec.country_code[i] = n[i];
    if(n.size() < 4) rec.country_code[n.size()] = '\0';

    rec.title = data::fixed_string<256>(title);
    rec.author = data::fixed_string<64>(author);
    rec.summary = data::fixed_string<256>("Summary text");
    rec.link = data::fixed_string<564>("https://example.com");
    return rec;
}

void create_mock_injector_file(const std::filesystem::path& path) {
    std::ofstream out(path, std::ios::binary);
    // Create 3 dummy records for the injector to read
    data::news_record r1 = make_record(1704067200, "Injected 1", "InjAuth", "USA", 1, 100);
    data::news_record r2 = make_record(1704067200, "Injected 2", "InjAuth", "IND", 2, 200);
    out.write(reinterpret_cast<char*>(&r1), sizeof(r1));
    out.write(reinterpret_cast<char*>(&r2), sizeof(r2));
    out.close();
}

// --- The Test Suites ---

void test_core_types() {
    std::cout << "\n--- RUNNING CORE TYPE TESTS ---\n";
    // 1. Memory Footprint Test (CRITICAL for binary I/O)
    EXPECT_TRUE(sizeof(data::news_record) == 1190, "news_record is exactly 1190 bytes");
    
    // 2. Fixed String Tests
    data::fixed_string<64> str1("Hello");
    data::fixed_string<64> str2("Hello");
    data::fixed_string<64> str3("World");
    
    EXPECT_TRUE(str1 == str2, "Fixed string equality operator works[cite: 39]");
    EXPECT_TRUE(!(str1 == str3), "Fixed string inequality works[cite: 39]");
}

void test_crud_and_persistence(const std::filesystem::path& db_path) {
    std::cout << "\n--- RUNNING CRUD & PERSISTENCE TESTS ---\n";
    
    // --- PHASE 1: Write ---
    {
        engine::record_manager api(db_path);
        EXPECT_TRUE(api.boot_up(), "Database cleanly booted (Phase 1)[cite: 36]");

        std::vector<data::news_record> initial_load;
        // Jan 1, 2024
        initial_load.push_back(make_record(1704067200, "Tech Apple Market", "Steve", "USA", 4, 500));
        initial_load.push_back(make_record(1704067200, "Finance Report", "Bill", "GBR", 2, 1000));
        // Feb 1, 2024
        initial_load.push_back(make_record(1706745600, "Apple Orchard Yields", "Farmer", "USA", 1, 200));

        // Inject IDs (Normally done by injector, we do manually for raw insert test)
        for(auto& r : initial_load) r.id = api.search_records({}).size() + 1; // dummy ids

        EXPECT_TRUE(api.insert_records(initial_load), "Bulk inserted 3 records successfully[cite: 36]");
        EXPECT_TRUE(api.shutdown(), "Database metadata saved and cleanly shut down[cite: 36]");
    }

    // --- PHASE 2: Read & Verify ---
    {
        engine::record_manager api(db_path);
        EXPECT_TRUE(api.boot_up(), "Database rebooted successfully (Persistence Check)[cite: 36]");

        engine::query_request req;
        req.start_time = 1704000000;
        req.end_time = 1707000000;
        
        // Test 1: Full Range Fetch
        auto results = api.search_records(req);
        EXPECT_TRUE(results.size() == 3, "Query engine fetched exact number of records from disk[cite: 36]");

        // Test 2: Text Filtering
        req.keyword = "Apple";
        req.search_title = true;
        auto text_results = api.search_records(req);
        EXPECT_TRUE(text_results.size() == 2, "Text filter correctly found 'Apple' in titles[cite: 36]");

        // Test 3: Sorting (Most Views)
        req.sort_by = engine::sort_order::MOST_VIEWS;
        auto sorted_results = api.search_records(req);
        EXPECT_TRUE(sorted_results[0].views == 500 && sorted_results[1].views == 200, "Introsort sorted by MOST_VIEWS correctly[cite: 36, 38]");

        // Test 4: Categorization Filter
        std::unordered_set<std::uint8_t> cats = {4}; // Only category 4
        std::unordered_set<std::string> nats; // Any nation
        api.categorize_records(cats, nats, sorted_results);
        EXPECT_TRUE(sorted_results.size() == 1 && sorted_results[0].category_id == 4, "Categorize filter dropped non-matching categories[cite: 36]");
        
        // --- PHASE 3: Update ---
        data::news_record rec_to_update = text_results[0];
        rec_to_update.views = 9999;
        EXPECT_TRUE(api.update_record(rec_to_update), "Record updated successfully[cite: 36]");

        // --- PHASE 4: Delete ---
        std::vector<data::news_record> dead_recs = { text_results[1] };
        EXPECT_TRUE(api.delete_records(dead_recs), "Record deleted successfully[cite: 36]");
        
        api.shutdown();
    }

    // --- PHASE 5: Verify Deletions/Updates Persisted ---
    {
        engine::record_manager api(db_path);
        api.boot_up();
        
        engine::query_request req;
        req.start_time = 1704000000;
        req.end_time = 1707000000;
        req.keyword = "Apple";
        req.search_title = true;
        
        auto results = api.search_records(req);
        EXPECT_TRUE(results.size() == 1, "Deleted record remained deleted after reboot[cite: 36]");
        EXPECT_TRUE(results[0].views == 9999, "Updated record view count persisted to disk[cite: 36]");
    }
}

void test_injector(const std::filesystem::path& db_path, const std::filesystem::path& injector_path) {
    std::cout << "\n--- RUNNING INJECTOR TESTS ---\n";
    create_mock_injector_file(injector_path);

    engine::record_manager api(db_path, injector_path);
    api.boot_up();

    EXPECT_TRUE(api.inject_records(), "Injector successfully read chunk data and passed to Facade");
    
    engine::query_request req;
    req.start_time = 1704000000;
    req.end_time = 1707000000;
    req.keyword = "Injected";
    req.search_title = true;
    
    auto results = api.search_records(req);
    EXPECT_TRUE(results.size() == 2, "Injected records successfully found in cold storage[cite: 36]");
    
    api.shutdown();
}

int main() {
    std::cout << "======================================\n";
    std::cout << " INITIATING BACKEND DIAGNOSTICS \n";
    std::cout << "======================================\n";

    // Setup secure sandbox directories
    std::filesystem::path test_db_path = std::filesystem::temp_directory_path() / "test_cold_db";
    std::filesystem::path test_injector_path = std::filesystem::temp_directory_path() / "test_injector.bin";

    // Clean start
    if (std::filesystem::exists(test_db_path)) std::filesystem::remove_all(test_db_path);
    if (std::filesystem::exists(test_injector_path)) std::filesystem::remove(test_injector_path);

    // Run tests
    test_core_types();
    test_crud_and_persistence(test_db_path);
    test_injector(test_db_path, test_injector_path);

    // Tear down
    std::filesystem::remove_all(test_db_path);
    std::filesystem::remove(test_injector_path);

    std::cout << "\n======================================\n";
    std::cout << " DIAGNOSTICS COMPLETE \n";
    std::cout << " Passed: " << tests_passed << "\n";
    std::cout << " Failed: " << tests_failed << "\n";
    std::cout << "======================================\n";

    return (tests_failed == 0) ? 0 : 1;
}