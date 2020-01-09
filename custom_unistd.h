//
// Created by root on 1/2/20.
//

#ifndef HEAP_MENAGER_CUSTOM_UNISTD_H
#define HEAP_MENAGER_CUSTOM_UNISTD_H

#include <unistd.h>
#include <cassert>
#include <cmath>
#include <mutex>

#define PAGE_SIZE       4096    // Długość strony w bajtach
#define MACHONE_WORD sizeof(void*)


#define SIZE_METADANE sizeof(memblock_t)

#define align4(x) (((((x)-1)>>2)<<2)+4)
#define align8(x) ((x + 7) & (-8))


typedef enum struct status{FREE, NOT_FREE} status;


typedef struct memblock_t
{
    int fence_start[4];
    memblock_t *next = nullptr;
    memblock_t *prev = nullptr;
    size_t size = 0;
    std::mutex lock;
    status status_ = status::NOT_FREE;
    void *data = nullptr; // wskźnik na dane
    size_t checksum{};
    int fence_end[4];
    void init_memblock();
}memblock_t;

typedef struct heap_menager{
    memblock_t *heap_head = nullptr; // wskźnik na pierwszy nieusuwalny element
    memblock_t *heap_tail = nullptr; // wskaźnik na osttni nie smiertelny element
    std::mutex lock;
    bool init = false;
}heap_menager;

static heap_menager heap_menager_;


void print_debug();

int heap_setup();

void* heap_malloc(size_t count);
void* heap_calloc(size_t number, size_t size);
void heap_free(void* memblock);
void* heap_realloc(void* memblock, size_t size);

memblock_t* find_block(size_t size);
memblock_t* fusion(memblock_t* block);

void split_block(memblock_t*, size_t);


void* custom_sbrk(intptr_t delta);

int heap_setup(void);
int extend_heap(size_t counter);


#if defined(sbrk)
#undef sbrk
#endif

#if defined(brk)
#undef brk
#endif


#define sbrk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), (void*)-1)
#define brk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), -1)

#endif //HEAP_MENAGER_CUSTOM_UNISTD_H
