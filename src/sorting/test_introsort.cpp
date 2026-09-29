#include <iostream>
// Include your master header (which includes the others)
#include "introsort.h" 
using namespace sort;

// A generic printer for our tests
template <typename T>
void print_array(T arr[], int size, const char* label) {
    std::cout << label << ": [ ";
    for (int i = 0; i < size; i++) {
        std::cout << arr[i] << (i < size - 1 ? ", " : " ");
    }
    std::cout << "]\n";
}

int main() {
    // ---------------------------------------------------------
    // TEST 1: Integers (Ascending Sort)
    // ---------------------------------------------------------
    int int_arr[] = {42, 7, -5, 12, 100, 0, 7, -22, 55};
    int int_size = sizeof(int_arr) / sizeof(int_arr[0]);
    
    print_array(int_arr, int_size, "Original Integers");
    
    // Sort Ascending using a Lambda
    intro_sort(int_arr, int_size, [](const int& a, const int& b) {
        return a < b; 
    });

    

    
    print_array(int_arr, int_size, "Sorted Integers (Asc)");
    std::cout << "---------------------------------------------------------\n";

    // ---------------------------------------------------------
    // TEST 2: Doubles (Descending Sort)
    // ---------------------------------------------------------
    double double_arr[] = {3.14, 2.71, -1.0, 9.81, 1.414, 0.0};
    int double_size = sizeof(double_arr) / sizeof(double_arr[0]);
    
    // Sort Descending (Notice we just change a < b to a > b)
    intro_sort(double_arr, double_size, [](const double& a, const double& b) {
        return a > b; 
    });
    
    print_array(double_arr, double_size, "Sorted Doubles (Desc)");
    std::cout << "---------------------------------------------------------\n";

    // ---------------------------------------------------------
    // TEST 3: Characters (Alphabetical)
    // ---------------------------------------------------------
    char char_arr[] = {'z', 'a', 'x', 'b', 'm', 'c', 'q'};
    int char_size = sizeof(char_arr) / sizeof(char_arr[0]);
    
    intro_sort(char_arr, char_size, [](const char& a, const char& b) {
        return a < b;
    });
    
    print_array(char_arr, char_size, "Sorted Chars (Asc)");
    std::cout << "---------------------------------------------------------\n";

    return 0;
}