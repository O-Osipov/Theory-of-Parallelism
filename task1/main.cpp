#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <numbers>

#define PI 3.1415926535 

#define USE_FLOAT 

#if defined(USE_FLOAT)
using arr_type = float;
#elif defined(USE_DOUBLE)
using arr_type = double;
#else
#error "Unknown type"
#endif


int main(){
    size_t size = 10000000;
    std::vector<arr_type> vec(size);
    arr_type sum = 0;
    std::cout << typeid(sum).name() << std::endl;
    for (int i=0; i < size; i++){
        arr_type val = (arr_type(i) * arr_type(2) * PI) / size;
        vec[i] = std::sin(val);
        sum += vec[i];
    }      
    std::cout << "Sum: " << sum;


    return 0;
}

