//
// Created by root on 1/2/20.
//

#include <cstdio>
#include "custom_unistd.h"

int heap_setup()
{
    assert(!heap_menager_.init);

    auto start_block = (memblock_t*)custom_sbrk(0);
    if ( custom_sbrk(PAGE_SIZE) == (void*)-1 ){
        printf("sbrk fail");
        return -1;
    }

    auto end_block = (memblock_t*)custom_sbrk(0) - 1;
    auto first_empty_block = start_block + 1;

    start_block->next = first_empty_block;
    first_empty_block->next = end_block;
    first_empty_block->prev = start_block;
    end_block->prev = first_empty_block;

    heap_menager_.heap_head = start_block;
    heap_menager_.heap_tail = end_block;
    heap_menager_.init = true;

//    tu jest blad !!!!!!!!!!!!1

    printf("%p %p %ld", start_block, (memblock_t*)custom_sbrk(0)-1,  (start_block-(memblock_t*)custom_sbrk(0)-1) );
    first_empty_block->size = start_block - (memblock_t*)custom_sbrk(0) - 1;

//

    return 0;
}


void print_debug()
{
    if ( heap_menager_.heap_head == nullptr )
        return;
//    printf("|%p %p %ld|", heap_menager_.heap_head, heap_menager_.heap_tail, heap_menager_.heap_tail-heap_menager_.heap_head);
    for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next) {
        printf("\n##############################\n");
        printf("\tAdres %p\n", iterator);
        printf("\tSize %zu\n", iterator->size);
        printf("\tfree %d\n", iterator->free);
        printf("##############################\n");
    }
}

