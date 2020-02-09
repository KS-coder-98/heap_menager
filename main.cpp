#include <cstdio>
#include "custom_unistd.h"
#include "Tests.h"


int main(int argc, char **argv)
{
//    test1();
//    test2();
//    test3();
//    test4();
//    test5();
    int status = heap_setup();
    assert(status == 0);

//    auto test =  getHead();
//    if (((intptr_t)test & (intptr_t)(PAGE_SIZE - 1)) == 0){
//        printf("tak");
//        printf("\n%p\n", test);
//    }
//    else{
//        printf("nie");
//    }

    assert(find_block_aligned_in_free_space(123));
    heap_dump_debug_information();
    heap_validate();
    return 0;
}
