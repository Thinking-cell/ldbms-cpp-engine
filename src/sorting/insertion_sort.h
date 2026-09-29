#pragma once

namespace sort{
namespace detail{
    template<typename T, typename Compare>
    void insertion_sort(T arr[], int left, int right, Compare comp){
        for(int sorted_index = left + 1; sorted_index <= right; sorted_index++){
            T key = arr[sorted_index];
            int check_index = sorted_index - 1;
            while(check_index >= left && comp(key, arr[check_index])){
                arr[check_index + 1] = arr[check_index];
                check_index--;
            }
            arr[check_index + 1] = key;
        }
    }
}
}
