
# News Record Manager (LDBMS)
### Low-Level Binary Linear Database Management System & Custom Introsort Engine
**Author:** Ranvir Singh | B.Tech CSE (LEET), Amritsar Group of Colleges (AGC)  
**Language:** Modern C++ (C++20) | **Build System:** CMake / Ninja / GCC 13+  
**Codebase:** 2,772 Lines of Code across 26 modular files | **Zero External Dependencies**

---

## 1. Project Overview & Philosophy

In third-semester computer science, Data Structures and Algorithms are frequently taught as whiteboard abstractions, while in production development, programmers often treat `std::vector`, `std::sort()`, and database engines as magical black boxes. 

The **News Record Manager (LDBMS)** was engineered from first principles to break those black boxes and explore fundamental systems programming:
* *What actually happens when a database writes bytes to a disk drive?*
* *How can structured records be queried without the massive CPU parsing overhead of CSV/JSON or the memory footprint of heavy SQL servers?*
* *How can custom data structures, hybrid sorting algorithms, and RAII transaction safety be implemented using direct memory mechanics?*

This system directly manages binary archive files on physical disk storage, providing an interactive console terminal for paginated browsing, time-series boundary search, in-place updates, and zero-fragmentation deletions.

---

## 2. Core Architectural Highlights

### A. Fixed-Stride Contiguous Binary Layout (POD Model)
To ensure that data in RAM and data on disk match byte-for-byte with zero serialization overhead, the core record entity is defined as a Plain Old Data (POD) struct with compiler padding eliminated:
```cpp
#pragma pack(push, 1)
struct news_record {
    uint64_t id;             // 8 bytes (Primary Key / Unique ID)
    uint64_t published_on;   // 8 bytes (Unix timestamp for publication)
    uint64_t created_on;     // 8 bytes (Unix timestamp for ingestion)
    uint64_t expires_at;     // 8 bytes (Unix timestamp for TTL retention)
    uint32_t views;          // 4 bytes (Numeric engagement metric)
    uint16_t category_id;    // 2 bytes (Master category enum key)
    uint16_t flags;          // 2 bytes (Bitmask: 0x01 = deleted, 0x02 = hot)
    
    fixed_string<256> title;   // 258 bytes (256B buffer + 2B length)
    fixed_string<64>  author;  // 66 bytes  (64B buffer + 2B length)
    fixed_string<564> link;    // 566 bytes (564B buffer + 2B length)
    fixed_string<256> summary; // 258 bytes (256B buffer + 2B length)
};
#pragma pack(pop)
// Total Record Stride: 40 + 258 + 66 + 566 + 258 = 1,188 Bytes
```

### B. Direct Stride Addressing ($O(1)$ Disk Access)
Because every record is strictly 1,188 bytes, finding record $k$ on disk requires zero sequential scanning:
$$\text{Byte Offset} = k \times 1,188$$
The storage manager jumps directly to that byte offset using `seekg`/`seekp`, enabling **instant random reads, writes, and in-place updates in $O(1)$ time**.

### C. Custom Data Structures Implemented from Scratch
1. **`td_array<T>` (Custom Dynamic Array):**
   * Built as an exploratory alternative to `std::vector`.
   * Implements geometric capacity doubling ($C \rightarrow 2C$) yielding **amortized $O(1)$ push-backs**.
   * Employs atomic `std::memcpy` for bulk buffer expansion, saturating system memory bandwidth (~25–50 GB/s) and bypassing C++ runtime object move loops.
2. **`simple_hash_map<K, V>` (Separate-Chaining Hash Table):**
   * Manages the 12 master news category mappings in $O(1)$ average time.
   * Modulo bucket distribution with singly-linked list collision resolution (Load factor $\alpha = 1.5$).
   * Copy/move constructors explicitly deleted (`= delete`) for static lifetime immutability.
3. **`fixed_string<N>` (Zero-Allocation String Buffer):**
   * Fixed-capacity stack buffer container with guaranteed null termination and overloaded `operator[]`.

### D. Hybrid Introsort Engine (David Musser)
Rather than relying on `std::sort()`, the engine implements David Musser’s hybrid **Introsort** (`introsort.h`) from scratch:
* **Primary Divide-and-Conquer:** Lomuto single-pass Quicksort partitioning for high-speed cache-friendly sorting.
* **Logarithmic Recursion Depth Limiter:** Recursion depth is tracked at each step ($2 \lfloor \log_2 N \rfloor$). If depth hits zero, the algorithm automatically branches to **Binary Heap Sort**, guaranteeing an **unbreakable $O(N \log N)$ worst-case time bound**.
* **Small-Partition Cutoff ($N < 16$):** Switches to cache-resident **Insertion Sort** for small subarrays, avoiding recursive function call overhead.

### E. LIFO Free-Space Hole Recycling Stack (`archive.holes`)
When records are deleted, the file is not rewritten. The engine logs the deleted record's byte offset onto a Last-In, First-Out (LIFO) stack (`archive.holes`). Subsequent record insertions pop from the hole stack and overwrite the vacated 1,188-byte slot in $O(1)$ time, **completely eliminating disk fragmentation**.

### F. Strong Exception Safety via RAII Transactions
Metadata and index updates are wrapped inside an inner RAII `transaction` class:
* Writes modifications to a temporary staging file (`.tmp`).
* Commits via an atomic filesystem rename (`std::filesystem::rename`).
* If an unhandled exception or power failure occurs before commit, the destructor automatically deletes the `.tmp` file, ensuring the database file is never corrupted.

### G. Single-Pass Two-Pointer In-Place Array Compaction
To scrub deleted records from the active memory viewing buffer, the UI uses a two-pointer compaction routine (`write_head` $\le$ `read_head`), eliminating the $O(M \cdot N)$ CPU penalty of calling `vec.erase()` in a loop.

---

## 3. Directory Structure

```
oop_alternate_sem3/
├── CMakeLists.txt              # CMake configuration
├── src/
│   ├── main.cpp                # Application entry point & OS signal handlers
│   ├── compile.bat             # Compiler automation script
│   ├── core/
│   │   ├── cold_data_types.h   # Enums, index_node, query DTOs
│   │   ├── fixed_string.h      # Fixed-capacity stack string buffer
│   │   ├── news_record.h       # Packed 1,188B POD struct
│   │   ├── simple_hash_map.h   # Custom separate-chaining hash table
│   │   └── td_array.h          # Custom dynamic array buffer
│   ├── csm/
│   │   ├── record_manager.h    # Facade interface coordinating subsystems
│   │   ├── record_manager.cpp  # Transaction lifecycle implementation
│   │   └── components/
│   │       ├── cold_storage_manager.h/.cpp    # Physical binary file I/O & holes
│   │       ├── historical_query_engine.h/.cpp # Time-series search & predicate filter
│   │       ├── id_gen.h/.cpp                  # Persistent 64-bit ID generator
│   │       ├── injector.h                     # Policy-based binary record injector
│   │       └── io_metadata_manager.h          # RAII transaction & serialization
│   ├── sorting/
│   │   ├── introsort.h         # Master hybrid Introsort dispatcher
│   │   ├── quick_sort.h        # Lomuto partitioning Quicksort
│   │   ├── heap_sort.h         # Zero-pointer Max-Heap & Floyd's heapify
│   │   ├── insertion_sort.h    # Small-partition Insertion sort
│   │   ├── swap.h              # In-place generic value swap
│   │   └── test_introsort.cpp  # Empirical sorting test harness
│   └── ui/
│       ├── terminal_app.h/.cpp     # Interactive console TUI & event loop
│       ├── terminal_renderer.h     # Word-wrapping & ANSI card formatter
│       └── terminal_utils.h        # Input parsing & sanitization
```

---

## 4. Compilation & Execution

### Prerequisites
* GCC / G++ supporting **C++20** (MinGW-w64 on Windows or GCC on Linux)
* CMake 3.20+ and Ninja (optional, or compile directly with `g++`)

### Fast Build via G++ (Windows / Linux):
```bash
g++ -std=c++20 -O3 -Wall -Wextra ^
    src/main.cpp ^
    src/csm/record_manager.cpp ^
    src/csm/components/cold_storage_manger.cpp ^
    src/csm/components/historical_query_manager.cpp ^
    src/csm/components/id_gen.cpp ^
    src/ui/terminal_app.cpp ^
    -I src ^
    -o bin/news_manager.exe
```

### Run Application:
```bash
./bin/news_manager.exe
```

### Run Empirical Sorting Test Suite:
```bash
g++ -std=c++20 -O3 src/sorting/test_introsort.cpp -I src -o bin/test_introsort.exe
./bin/test_introsort.exe
```

---

## 5. Course Alignment

Developed for **Data Structures & Algorithms Lab (AGCS-25307)** and **Object-Oriented Programming Lab (AGCS-25306)** at Amritsar Group of Colleges (AGC).
* **CO1:** Custom dynamic arrays and heap management (`td_array.h`).
* **CO2:** Implicit binary heap (`heap_sort.h`) and separate-chaining hash tables (`simple_hash_map.h`).
* **CO3:** Hybrid comparison-based Introsort (`introsort.h`).
* **CO4:** Logarithmic binary search on chronological `.idx` indices.
* **CO5:** In-place two-pointer compaction and RAII transaction rollback.
* **CO6:** Complete synthesis of systems architecture into a functional binary database engine.
