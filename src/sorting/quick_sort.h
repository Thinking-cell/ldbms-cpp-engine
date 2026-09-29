#pragma once
#include "swap.h"


namespace sort{
namespace detail{
    template<typename T, typename Compare>
    int chop_array(T arr[], int left, int right, Compare comp){
        T pivot = arr[right];
        int boundary = left - 1;
        for(int scanner = left; scanner < right; scanner++){
            if(comp(arr[scanner], pivot)){
                boundary++;
                swap_elements(arr[scanner], arr[boundary]);
            }
        }

        swap_elements(arr[boundary + 1], arr[right]);
        return boundary + 1;
    }

    template<typename T, typename Compare>
    void quick_sort(T arr[], int left, int right, Compare comp){
        if(left >= right || right < 0){
            return;
        }

        int pivot_index = chop_array(arr, left, right, comp);

        quick_sort(arr, left, pivot_index - 1, comp);

        quick_sort(arr, pivot_index + 1, right, comp);

    }

}
}
