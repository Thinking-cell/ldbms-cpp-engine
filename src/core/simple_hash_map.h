#pragma once
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

namespace data {

    /**
     * simple separate chaining hash map container to store and check categories
     */
    template<typename K, typename V, std::uint32_t BUCKET_COUNT = 8>
    class simple_hash_map {
    
        // linked list node for resolving separate chaining collision 
        struct node {
            K key;
            V value;
            node* next;

            node(const K& k, const V& v, node* n = nullptr) : key(k), value(v), next(n) {}
        };

        node* buckets[BUCKET_COUNT]{}; // initialize the array with nullptrs
        std::uint32_t current_size;

        // get bucket index from key using standard hash
        std::uint32_t get_bucket_index(const K& key) const {
            return static_cast<std::uint32_t>(std::hash<K>{}(key) % BUCKET_COUNT);
        }

        // deallocate all nodes in the buckets
        void clear() {
            for (std::uint32_t i = 0; i < BUCKET_COUNT; ++i) {
                node* curr = buckets[i];
                while (curr != nullptr) {
                    node* temp = curr;
                    curr = curr->next;
                    delete temp;
                }
                buckets[i] = nullptr;
            }
            current_size = 0;
        }

    public:
        // default constructor
        simple_hash_map() : current_size(0) {}

        // initializer list constructor 
        // initializing Master categories with this in news_record.h
        simple_hash_map(std::initializer_list<std::pair<K, V>> i_list) : current_size(0) {
            for (const auto& item : i_list) {
                insert(item.first, item.second);
            }
        }

        // destructor to free all allocated nodes
        ~simple_hash_map() {
            clear();
        }

        // banning copying moving and assignment of the hash
        // dont need it for its implementation
        simple_hash_map(const simple_hash_map&) = delete;
        simple_hash_map& operator=(const simple_hash_map&) = delete;
        simple_hash_map(simple_hash_map&& other) = delete;
        simple_hash_map& operator=(simple_hash_map&& other) = delete;

        // insert or update key value pair
        void insert(const K& key, const V& value) {
            std::uint32_t index = get_bucket_index(key);
            node* curr = buckets[index];

            // update value if key already exists in bucket chain
            while (curr != nullptr) {
                if (curr->key == key) {
                    curr->value = value;
                    return;
                }
                curr = curr->next;
            }

            // prepend new node at head of chain and setting next to head
            buckets[index] = new node(key, value, buckets[index]);
            current_size++;
        }

        // read only find method returning pointer to value or nullptr
        const V* find(const K& key) const {
            std::uint32_t index = get_bucket_index(key);
            node* curr = buckets[index];
            // loop till values is found
            while (curr != nullptr) {
                if (curr->key == key) {
                    return &curr->value;
                }
                curr = curr->next;
            }
            // return if no value found in the chain
            return nullptr;
        }

        // accessible find method returning pointer to value or nullptr
        V* find(const K& key) {
            std::uint32_t index = get_bucket_index(key);
            node* curr = buckets[index];
            // loop till values is found
            while (curr != nullptr) {
                if (curr->key == key) {
                    return &curr->value;
                }
                curr = curr->next;
            }
            // return if no value found in the chain
            return nullptr;
        }


        // check if key exists in hash map
        bool contains(const K& key) const {
            return find(key) != nullptr;
        }

        // return number of elements in hash map
        std::uint32_t size() const {
            return current_size;
        }

        // return number of buckets in hash map
        std::uint32_t bucket_count() const {
            return BUCKET_COUNT;
        }

        // check if hash map is empty
        bool empty() const {
            return current_size == 0;
        }
    };

}
