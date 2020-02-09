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
    int line = -1;
    char *name_file;
    status status_ = status::NOT_FREE;
    void *data = nullptr; // wskźnik na dane
    size_t checksum;
    int fence_end[4];
    void init_memblock();
    size_t count_checksum();
    size_t set_checksum();
}memblock_t;

typedef struct heap_menager{
    memblock_t *heap_head = nullptr; // wskźnik na pierwszy nieusuwalny element
    memblock_t *heap_tail = nullptr; // wskaźnik na osttni nie smiertelny element
    std::mutex lock;
    bool init = false;
}heap_menager;

static heap_menager heap_menager_;

memblock_t* getHead();

void heap_dump_debug_information();

int heap_setup();

void* heap_malloc(size_t count);
void* heap_calloc(size_t number, size_t size);
void heap_free(void* memblock);
void* heap_realloc(void* memblock, size_t size);

void test_linked_list();

memblock_t* find_block(size_t size);
memblock_t* fusion(memblock_t* block);

void split_block(memblock_t*, size_t);


void* heap_malloc_debug(size_t count, int fileline, const char* filename);
void* heap_calloc_debug(size_t number, size_t size, int fileline, const char* filename);
void* heap_realloc_debug(void* memblock, size_t size, int fileline, const char* filename);

size_t heap_get_used_space(void);
size_t heap_get_largest_used_block_size(void);
uint64_t heap_get_used_blocks_count(void);
size_t heap_get_free_space(void);
size_t heap_get_largest_free_area(void);
uint64_t heap_get_free_gaps_count(void);

void* custom_sbrk(intptr_t delta);

enum pointer_type_t{
    pointer_null, //przekazany wskaźnik jest pusty – posiada wartość NULL
    pointer_out_of_heap, // przekazany wskaźnik nie leży w obszarze sterty.
    pointer_control_block, // przekazany wskaźnik leży w obszarze sterty, ale wskazuje na obszar struktur wewnętrznych.
    pointer_inside_data_block, // przekazany wskaźnik wskazuje środek jakiegoś zaalokowanego bloku
    pointer_unallocated, // przekazany wskaźnik wskazuje na obszar wolny (niezaalokowany).
    pointer_valid // przekazany wskaźnik jest poprawny
};

enum pointer_type_t get_pointer_type(const void* pointer);

int extend_heap(size_t counter);
bool reduce_heap();

int heap_validate(void);

void* heap_get_data_block_start(const void* pointer);

size_t heap_get_block_size(const void* memblock);

void* heap_malloc_aligned(size_t count);
void* heap_calloc_aligned(size_t number, size_t size);
void* heap_realloc_aligned(void* memblock, size_t size);
memblock_t* find_block_aligned_in_free_space(size_t size);
memblock_t* find_block_aligned(size_t size);
memblock_t* find_block_aligned_in_free_space_to_realloc(memblock_t* block, size_t size);

#if defined(sbrk)
#undef sbrk
#endif

#if defined(brk)
#undef brk
#endif


#define sbrk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), (void*)-1)
#define brk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), -1)

#endif //HEAP_MENAGER_CUSTOM_UNISTD_H
