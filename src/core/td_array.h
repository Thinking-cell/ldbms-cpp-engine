#pragma once
#include <cstdint>
#include <cstring>
#include <vector>


namespace data{
    /**
     * 
     */
    template<typename T>
    class td_array{
        T* data;
        std::uint32_t current_size;
        std::uint32_t capacity;
        
        /**
         * atomic copy overloaded methods returning pointer
         */
        // returns the ptr to a copy of data inputted by td_array
        T* atomic_copy_data(const td_array& other){
            if(other.current_size == 0 || other.data == nullptr)
                return nullptr;
            std::uint32_t input_size = other.current_size;
            // reserve input size on heap
            T* copy_ptr = new T[input_size];

            // copying using low level atomic memory copy
            std::memcpy(copy_ptr, other.data, input_size * sizeof(T));

            return copy_ptr;

        }

         // returns the ptr to a copy of data inputted by ptr and number of objects in the array
        T* atomic_copy_data(const T* data_ptr, size_t d_size){
            if(d_size == 0 || data_ptr == nullptr)
                return nullptr;
            std::uint32_t input_size = d_size;
            // reserve input size on heap
            T* copy_ptr = new T[input_size];

            // copying using low level atomic memory copy
            std::memcpy(copy_ptr, data_ptr, input_size * sizeof(T));

            return copy_ptr;

        }

        // expand the td_array with set amount
        void expand_size(std::uint32_t new_size){
            if (new_size <= capacity || new_size == 0)
                return;

            // allocate the new data on heap
            T* new_data = new T[new_size];
            // atomic copy old data
            std::memcpy(new_data, data, current_size * sizeof(T));
            // delete old data
            delete[] data;
            // set new data
            data = new_data;
            // update capacity
            capacity = new_size;
            // abolish new_data pointer
            new_data = nullptr;

        }

        // trim the td_array, will trim data as well if trim size < current size
        void shrink_size(std::uint32_t new_size){
            if (new_size >= capacity)
                return;
            
            // allocate the new data on heap
            T* new_data = new T[new_size];
            // atomic copy old data
            std::memcpy(new_data, data, ((new_size < current_size) ? new_size : current_size) * sizeof(T));
            // delete old data
            delete[] data;
            // set new data
            data = new_data;
            // update capacity
            capacity = new_size;
            // update current size if trim size was lower than current size
            if(new_size < current_size)
                current_size = new_size;
            // abolish new_data pointer
            new_data = nullptr;
            
            
        }

    
    public:

        // default constructor
        td_array() : data(nullptr), current_size(0), capacity(0){}
        // initialize the td_array
        td_array(std::uint32_t s) : current_size(0), capacity(s){
            data = (capacity > 0) ? new T[capacity]: nullptr;
        }

        // destructor
        ~td_array(){
            delete[] data;
        }

        
        // move semantics 
        td_array(td_array&& other) : data(other.data), current_size(other.current_size), capacity(other.capacity) {
            other.data = nullptr;
            other.current_size = 0;
            other.capacity = 0;
        }

        
        // copy constructor
        td_array(const td_array& other) : current_size(other.current_size), capacity(other.capacity) {
            data = atomic_copy_data(other);
        }
        
        // copy assignment overload
        td_array& operator=(const td_array& other){
            // no self assignment
            if (this == &other)
                return *this;
            // delete old data
            delete[] data;
            data = atomic_copy_data(other);
            current_size = other.current_size;
            capacity = other.capacity;
            return *this;
        }
        // copy assignment operator overloading with move semantics
        td_array& operator=(td_array&& other){
            if (this == &other)
                return *this;

            delete[] data;
            data = other.data;
            current_size = other.current_size;
            capacity = other.capacity;
            other.data = nullptr;
            other.current_size = 0;
            other.capacity = 0;
            return *this;
        }

         // copy assignment overload for vector
        td_array& operator=(const std::vector<T>& other){
            // delete old data
            delete[] data;
            data = (other.size() > 0) ? atomic_copy_data(other.data(), other.size()) : nullptr;
            current_size = other.size();
            capacity = other.size();
            return *this;
        }
        
        // to convert the td_array back to vector
        std::vector<T> to_vector() const {
            if (current_size == 0 || data == nullptr) return {};
            return std::vector<T>(data, data + current_size);
        }
        // reserve atleast size number of objects in the td_array
        void reserve(std::uint32_t size){
            if (size <= capacity)
                return;
            expand_size(size);
        }
        // resize the td_array with given size
        void resize(std::uint32_t size){
            if(size < capacity)
                shrink_size(size);
            
            if(size > capacity)
                expand_size(size);
        }


        // append an object in the td_array
        void push_back(T& item){
            if (current_size >= capacity){
                reserve(((capacity > 0) ? capacity : 8) * 2);
            }
            
            *(data + current_size) = item;
            current_size++;
        }
        // append using move semantics or R value
        void push_back(T&& item){
            if (current_size >= capacity)
                reserve(((capacity > 0) ? capacity : 8) * 2);
            
            *(data + current_size) = std::move(item);
            current_size++;
        }

        void set_currentsize(std::uint32_t new_size){
            if(new_size > capacity)
                return;
            current_size = new_size;
        }
        // overloading random access operator to return read only value
        const T& operator[](int index) const {
            return *(data + index);
        }

        // overloading random access operator to return accessible value
        T& operator[](int index){
            return *(data + index);
        }
        
        // return current size of the td_array
        std::uint32_t size() const {
            return current_size;
        }

        // return max capacity of the td_array
        std::uint32_t max_size() const {
            return capacity;
        }
        // returns a constant pointer to data
        const T* get_data() const{
            return data;
        }

        // returns accessible pointer to data
        T* get_data(){
            return data;
        }

        // accessible begin method for compiler to do range based loop
        T* begin(){
            return data;
        }

        // accessible end method for compiler to do range based loop
        T* end(){
            return data + current_size; // return the address just after the last element
        }

        // read only begin method for compiler to do range based loop
        const T* begin() const { 
            return data; 
        }
        // read only end method for compiler to do range based loop
        const T* end() const {
            return data + current_size; 
        }

        // checks if the array is empty
        bool empty() const{
            return current_size == 0;
        }



    };
}
