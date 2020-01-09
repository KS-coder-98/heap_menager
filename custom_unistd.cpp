//
// Created by root on 1/2/20.
//

#include <cstdio>
#include <cstring>
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

    return 0;
}


void print_debug()
{
    if ( heap_menager_.heap_head == nullptr )
        return;
    printf("Size mataData %lu\n", SIZE_METADANE);
    for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next) {
        printf("\n##############################\n");
        printf("\tAddress %p\n", iterator);
        printf("\tData address %p\n", iterator->data);
        printf("\tSize %zu\n", iterator->size);
        if ( iterator->status_ == status::FREE )
            printf("\tIs free Yes\n");
        else
            printf("\tIs free No\n");
        printf("##############################\n");
    }
}


void memblock_t::init_memblock() {
    for ( size_t i = 0; i<4; i++){
        fence_start[i] = 9;
        fence_end[i] = 9;
    }
    data = this + 1;
    status_ = status::NOT_FREE;
}

void* heap_malloc(size_t count)
{
    memblock_t *new_block = nullptr;//, *last;
    size_t s;
    if (sizeof(void*) == 8 )
        s = align8(count);
    else
        s = align4(count);
    if ( heap_menager_.init ){
        new_block = find_block(s);
        if ( new_block ){ //jesli znajdzie wystarczajaco duzy blok
            //czy mozna podzielic blok
            new_block->init_memblock();
            new_block->status_ = status::NOT_FREE;
            if ( (new_block->size - s) >= (SIZE_METADANE + sizeof(void*)) )
                split_block(new_block, s);
            new_block->status_ = status::NOT_FREE;
        }
        else{ // jesli nie znajdzie wystarczjaco duzego bloku
            //todo
            int error = extend_heap(count);
            if ( error == -1 ){
                return NULL;
            }
            else if ( error == 0 ){
                return heap_malloc(count);
            }
        }
    }
    else{
        heap_setup();
        return heap_malloc(s);
    }
    new_block->data = new_block+1;
    return ( new_block->data );
};
void* heap_calloc(size_t number, size_t size)
{
    auto temp1 = (memblock_t*)heap_malloc(number*size) - (intptr_t)SIZE_METADANE;
    memset(temp1 + SIZE_METADANE, 0, temp1->size);
};
void heap_free(void* block)
{
    auto temp = (memblock_t*)block - 1;
    temp->status_= status::FREE;
    fusion(temp);
};
void* heap_realloc(void* block, size_t size)
{
    if ( !block )
        return malloc(size);
    auto temp_block = (memblock_t*)block - 1;//todo sprawdzic to
    if ( sizeof(void*) == 8 )
        size = align8(size);
    else
        size = align4(size);
    //jesli rozmiar w reaaloc jest mnejszy lub rowny zadanemu
    if ( temp_block->size >= size){
        if ( temp_block->size - size >= SIZE_METADANE + MACHONE_WORD )
            split_block(temp_block, size);
    }
    else{ //czyli powiekszamy
        //nastepne blok jest wolny i ma wystarczjaco duzo mjesca
        if ( temp_block->next && temp_block->next->status_ == status::FREE && temp_block->size + SIZE_METADANE + temp_block->next->size >= size){
            fusion(temp_block);
            if ( temp_block->size >= SIZE_METADANE + MACHONE_WORD ){
                split_block(temp_block, size);
            }
        }
        else{ //realoc a alokacja nowego bloku
            auto new_block = (memblock_t*)malloc(size);
            if ( !new_block )
                return nullptr;
            new_block->init_memblock();
            temp_block->status_ = status::FREE;
            fusion(temp_block);
            return memcpy(new_block->data, temp_block->data, temp_block->size);
        }
    }
    return temp_block;
};

memblock_t* find_block(size_t size)
{
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
    new_block = (memblock_t*)((intptr_t)block->data + s);
    new_block->init_memblock();
    new_block->size = block->size - s - SIZE_METADANE;
    new_block->next = block->next;
    new_block->status_ = status::FREE;
    block->size = s;
    block->next = new_block;
    new_block->prev = block;
    new_block->status_ = status::FREE;
}

memblock_t* fusion(memblock_t* block)
{
    //check next
    if ( block->next && block->next->status_==status::FREE ){
        block->size += SIZE_METADANE + block->next->size;
        block->next = block->next->next;
        block->next->prev = block;
    }

    //check prev
    if ( block->prev && block->prev->status_==status::FREE ){
        block->prev->size += SIZE_METADANE + block->size;
        block->prev->next = block->next;
        block->next->prev = block->prev;
    }
    return block;
}

int extend_heap(size_t counter)
{
    int number_of_pages_neeeded;
    number_of_pages_neeeded = ceil((double)counter/(double)PAGE_SIZE);
    auto old_end = (memblock_t*)custom_sbrk(0) - 1;
    if ( custom_sbrk(number_of_pages_neeeded * PAGE_SIZE) == (void*)-1 ){
        /* sbrk fail*/
        return -1;
    }
    auto new_end_block = (memblock_t*)custom_sbrk(0) - 1;
    new_end_block->init_memblock();
    new_end_block->next = nullptr;
    new_end_block->prev = old_end;

    old_end->next = new_end_block;
    old_end->status_ = status::FREE;
    old_end->size += PAGE_SIZE - SIZE_METADANE;
    //tu jest blad ze wskzniki bloku old end wskzuje na poczatek pierwszej strony !!!!
    fusion(old_end); //chceck

    heap_menager_.heap_tail = new_end_block;
    return 0;
}