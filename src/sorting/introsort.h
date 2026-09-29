#pragma once
#include<cmath>
#include "heap_sort.h"
#include "insertion_sort.h"
#include "quick_sort.h"


namespace sort{
namespace detail{

    const int INSERTION_SORT_THRESHOLD = 16;

    // this is basically a quick sort with conditional insertion
    // and heap sort to mitigate quick sort's worst case scenario
    // if the array gets too small or recursion is hit too many times
    /**
     * @addtogroup sorting_algorithms
     * @brief Implements the Introsort algorithm, which combines Quick Sort, Heap Sort, and Insertion Sort for efficient sorting.
     * 
     * @param arr array to be sorted
     * @param left left boud
     * @param right right bound
     * @param depth_limit max allowed recursion depth
     * @param comp comparator function for the data in array
     */
    template<typename T, typename Compare>
    void intro_sort_recursive(T arr[], int left, int right, int depth_limit, Compare comp){
        int size = right - left + 1;

        // BASE case 1: intro sort rule 1, if the chunk to be sorted is less
        // than insertion sort threshold then just use insertion sort
        if(size < INSERTION_SORT_THRESHOLD){
            insertion_sort(arr, left, right, comp);
            return;
        }

        // BASE case 2: intro sort rule 2, if recursion is used more than the allowed
        // depth_limit for sorting the array then default to heap sort for the
        // entire array to guarantee nlogn time complexity
        if(depth_limit == 0){
            // passing only the unsorted array to heap sort
            // using pointer arithematics
            heap_sort(arr + left, size, comp);
            return;
        }

        // intro sort rule 3, using quick sort partitioning
        // to sort the array by default
        int pivot = chop_array(arr, left, right, comp);

        // now sort the other halves using recursion
        intro_sort_recursive(arr, left, pivot - 1, depth_limit - 1, comp);
        intro_sort_recursive(arr, pivot + 1, right, depth_limit - 1, comp);

    }
}

/**
 * @addtogroup sorting_algorithms
 * @brief Implements the Introsort algorithm, which combines Quick Sort, Heap Sort, and Insertion Sort for efficient sorting.
 * 
 * @param arr array to be sorted
 * @param size size of the array
 * @param comp comparator function for the data in array
 */
template<typename T, typename Compare>
void intro_sort(T arr[], int size, Compare comp){
    if(size <= 1)
        return;
    // depth_limit is 2 * log(n), as per standard introsort implementation
    detail::intro_sort_recursive(arr, 0, size - 1, 2 * (std::log2(size)), comp);
}

}

