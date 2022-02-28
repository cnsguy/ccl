// Doubles as a C++ build test

#include "int_map_test.hpp"
#include "string_map_test.hpp"
#include <ctime>
#include <cstdlib>
#include <iostream>

int main()
{
    srand((unsigned int) time(NULL));
    int_map_test_run();
    string_map_test_run();
    return 0;
}
