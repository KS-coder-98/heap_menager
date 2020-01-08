//
// Created by root on 1/2/20.
//

#include <cstdio>
#include "custom_unistd.h"

int heap_setup()
{
    assert(!heap_menager_.init);

    memblock_t * start_block = (memblock_t *)custom_sbrk(0);
    if ( custom_sbrk(PAGE_SIZE) == (void*)-1 ){
        printf("sbrk fail");
        return -1;
    }

    memblock_t* end_block = (memblock_t*)custom_sbrk(0) - 1;
    memblock_t* first_empty_block = start_block + 1;

    start_block->next = first_empty_block;
    first_empty_block->next = end_block;
    first_empty_block->prev = start_block;
    end_block->prev = first_empty_block;

    heap_menager_.heap_head = (memblock_t*)start_block;
    heap_menager_.heap_tail = (memblock_t*)end_block;
    heap_menager_.init = true;

    first_empty_block->size = (intptr_t)custom_sbrk(0) - (intptr_t)start_block - 3 * SIZE_METADANE;

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

