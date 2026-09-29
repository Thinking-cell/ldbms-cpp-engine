#pragma once
#include "swap.h"

namespace sort{
namespace detail{
    template<typename T, typename Compare>
    void heapify(T arr[], int parent, int size, Compare comp){
        int left_child = 2 * parent + 1;
        int right_child = 2 * parent + 2;
        int largest = parent;

        if(left_child < size && comp(arr[largest], arr[left_child])){
            largest = left_child;
        }
        if(right_child < size && comp(arr[largest], arr[right_child])){
            largest = right_child;
        }
        if(parent != largest){
            swap_elements(arr[parent], arr[largest]);
            heapify(arr, largest, size, comp);
        }
    }

    template<typename T, typename Compare>
    void heap_sort(T arr[], int size, Compare comp){

        int last_parent = (size - 1) / 2;
        // Heapify every parent node
        for(int j = last_parent; j >= 0; j--){
            heapify(arr, j, size, comp);
        }

        // harvesting the top node and the heapifying that parent node
        // to balance the tree again for the swapped node
        for(int i = size - 1; i > 0; i--){
            swap_elements(arr[0], arr[i]);
            heapify(arr, 0, i, comp);
        }
    }
}
}
