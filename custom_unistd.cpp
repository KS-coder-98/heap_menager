//
// Created by root on 1/2/20.
//

#include <cstdio>
#include "custom_unistd.h"

int heap_setup()
{
    assert(!heap_menager_.init);

    auto *start_block = (memblock_t *)custom_sbrk(0);
    start_block->init_memblock();
    if ( custom_sbrk(PAGE_SIZE) == (void*)-1 ){
        printf("sbrk fail");
        return -1;
    }

    auto end_block = (memblock_t*)custom_sbrk(0) - 1;
    end_block->init_memblock();
    auto first_empty_block = start_block + 1;
    first_empty_block->init_memblock();

    start_block->next = first_empty_block;
    first_empty_block->next = end_block;
    first_empty_block->prev = start_block;
    end_block->prev = first_empty_block;

    heap_menager_.heap_head = (memblock_t*)start_block;
    heap_menager_.heap_tail = (memblock_t*)end_block;
    heap_menager_.init = true;

    first_empty_block->size = (intptr_t)custom_sbrk(0) - (intptr_t)start_block - 3 * SIZE_METADANE;
    first_empty_block->status_ = status::NOT_FREE;
    return 0;
}


void print_debug()
{
    if ( heap_menager_.heap_head == nullptr )
        return;
    for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next) {
        printf("\n##############################\n");
        printf("\tAddress %p\n", iterator);
        printf("\tSize %zu\n", iterator->size);
        if ( iterator->status_ == status::NOT_FREE )
            printf("\tIs free Yes\n");
        else
            printf("\tIs free No\n");
/*
        for ( size_t  i = 0; i < 4; i++){
            printf("%d ", iterator->fence_start[i]);
            if ( i == 3 )
                printf("\n");
        }
  */
        printf("##############################\n");
    }
}


void memblock_t::init_memblock() {
    for ( size_t i = 0; i<4; i++){
        fence_start[i] = 9;
        fence_end[i] = 9;
    }
}
