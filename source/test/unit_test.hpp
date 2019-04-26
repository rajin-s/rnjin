#ifndef TEST
    #define unit_test(name) int name()
    #define finish() return 0
    #define for_test(file) file
#else
    #define run int
    #define unit_test(name) name = 0;
    #define finish()
    #define for_test(file) "nothing.h"
#endif