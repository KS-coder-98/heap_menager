//
// Created by root on 1/2/20.
//

#ifndef HEAP_MENAGER_CUSTOM_UNISTD_H
#define HEAP_MENAGER_CUSTOM_UNISTD_H

#include <unistd.h>
#include <cassert>

#define PAGE_SIZE       4096    // Długość strony w bajtach

#define SIZE_METADANE sizeof(memblock_t)

#define align4(x) (((((x)-1) >>2)<<2)+4)

typedef struct memblock_t
{
    int fence_start[4] = {9, 9, 9, 9};
    memblock_t *next = nullptr;
    memblock_t *prev = nullptr;
    size_t size = 0;
    bool free = true; // true - is free; false - is not free
    void *ptr; // wskźnik na dane
    size_t checksum;
    int fance_end[4] = {9, 9, 9, 9};
}memblock_t;

typedef struct heap_menager{
    memblock_t *heap_head = nullptr; // wskźnik na pierwszy nieusuwalny element
    memblock_t *heap_tail = nullptr; // wskaźnik na osttni nie smiertelny element
    bool init = false;
}heap_menager;

static heap_menager heap_menager_;


void print_debug();


int heap_setup();



void* custom_sbrk(intptr_t delta);

int heap_setup(void);


#if defined(sbrk)
#undef sbrk
#endif

#if defined(brk)
#undef brk
#endif


#define sbrk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), (void*)-1)
#define brk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), -1)

#endif //HEAP_MENAGER_CUSTOM_UNISTD_H
