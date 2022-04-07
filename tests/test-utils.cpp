#include <iostream>

#define UTILS_RUN_TESTS
#include "../my_utils.hpp"

int main(){
    try{
    my::run_all_tests();
    }catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        exit (-1);
    }

    return 0;
}