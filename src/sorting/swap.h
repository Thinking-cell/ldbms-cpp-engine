#pragma once


namespace sort{
namespace detail{
    template<typename T>
    void swap_elements(T& a, T&b){
        T temp = a;
        a = b;
        b = temp;
    }
}
}
