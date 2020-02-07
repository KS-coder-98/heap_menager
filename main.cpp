#include <cstdio>
#include "custom_unistd.h"
#include "Tests.h"


int main(int argc, char **argv)
{
//    test1();
//    test2();
//    test3();
//    test4();
    test5();
    heap_validate();
    heap_dump_debug_information();
    return 0;
}
