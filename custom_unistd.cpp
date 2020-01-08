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
    first_empty_block->status_ = status::FREE;
    first_empty_block->data = first_empty_block + SIZE_METADANE;
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

void* heap_malloc(size_t count)
{
    memblock_t* new_block;//, *last;
    size_t s =align4(count) ;
    if ( heap_menager_.init ){
//        last = heap_menager_.heap_head;
        new_block = find_block(s);
//        new_block
        if ( new_block ){ //jesli znajdzie wystarczajaco duzy blok
            //czy mozna podzielic blok
            new_block->status_ = status::NOT_FREE;
            if ( (new_block->size - s) >= (SIZE_METADANE + 4) )
                split_block(new_block, s);
            new_block->status_ = status::NOT_FREE;
        }
        else{ // jesli nie znajdzie wystarczjaco duzego bloku
            //todo
        }
    }
    else{
        //first innit;
        printf("tuuu");
    }
    new_block->data = new_block+SIZE_METADANE;
    return ( new_block->data );
};
void* heap_calloc(size_t number, size_t size)
{

};
void heap_free(void* memblock)
{
    return;
};
void* heap_realloc(void* memblock, size_t size)
{

};

memblock_t* find_block(size_t size)
{
    /*
    memblock_t* b = heap_menager_.heap_head;
    auto last =b;
    while ( b && !(b->status_ == status::FREE && b->size >= size) ){
        last = b;
        b = b->next;
    }
    last->data = last + SIZE_METADANE;
    return last;
     */
    auto temp_block = heap_menager_.heap_head;
    while ( temp_block ){
        if ( temp_block->size >= size + SIZE_METADANE  && temp_block->status_ == status::FREE){
            return temp_block;
        }
        temp_block = temp_block->next;
    }
    return nullptr;
}

void split_block(memblock_t* block, size_t s)
{
    memblock_t* new_block;
    assert(block->data != nullptr);
//    new_block = reinterpret_cast<memblock_t *>((intptr_t) block->data + s);
    new_block = (memblock_t*)((intptr_t)block->data + s);
    new_block->size = (intptr_t)block->size - s - 4;
    new_block->next = block->next;
    new_block->status_ = status::FREE;
    block->size = s;
    block->next = new_block;

    new_block->prev = block;

}