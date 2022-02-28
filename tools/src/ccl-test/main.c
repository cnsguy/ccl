#include "int_array_test.h"
#include "string_array_test.h"
#include "int_map_test.h"
#include "string_map_test.h"
#include "string_int_map_test.h"
#include "int_string_map_test.h"
#include "ccl/ccl_bool.h"
#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    srand((unsigned int) time(NULL));

    int_array_test_run(0, 0);
    string_array_test_run(0, 0);
    int_map_test_run(0, 0);
    string_int_map_test_run(0, 0);
    int_string_map_test_run(0, 0);
    string_map_test_run(0, 0);

    int_array_test_run(1, rand() % 1000);
    string_array_test_run(1, rand() % 1000);
    int_map_test_run(1, rand() % 1000);
    string_int_map_test_run(1, rand() % 1000);
    int_string_map_test_run(1, rand() % 1000);
    string_map_test_run(1, rand() % 1000);

    int_array_test_run(1, 0);
    string_array_test_run(1, 0);
    int_map_test_run(1, 0);
    string_int_map_test_run(1, 0);
    int_string_map_test_run(1, 0);
    string_map_test_run(1, 0);

    return 0;
}
