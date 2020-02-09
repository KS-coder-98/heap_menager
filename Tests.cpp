//
// Created by root on 2/7/20.
//

#include "Tests.h"

void test1()
{
    int status = heap_setup();
    assert(status == 0);

    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void* p1 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p2 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p3 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p4 = heap_malloc(45 * 1024 * 1024); // 45MB
    assert(p1 != NULL); // malloc musi się udać
    assert(p2 != NULL); // malloc musi się udać
    assert(p3 != NULL); // malloc musi się udać
    assert(p4 == NULL); // nie ma prawa zadziałać

    // Ostatnia alokacja, na 45MB nie może się powieść,
    // ponieważ sterta nie może być aż tak
    // wielka (brak pamięci w systemie operacyjnym).

    status = heap_validate();
    assert(status == 0); // sterta nie może być uszkodzona

    // zaalokowano 3 bloki
    assert(heap_get_used_blocks_count() == 3);

    // zajęto 24MB sterty; te 2000 bajtów powinno
    // wystarczyć na wewnętrzne struktury sterty
    assert(
            heap_get_used_space() >= 24 * 1024 * 1024 &&
            heap_get_used_space() <= 24 * 1024 * 1024 + 2000
    );

    // zwolnij pamięć
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

    // już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
}

void test2(){
    int status = heap_setup();
    assert(status == 0);

    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    // parametry pustej sterty
    int const size_tab = 1000;
    int *tab = (int*)heap_malloc(sizeof(int)*size_tab);
    assert( get_pointer_type(tab) == pointer_valid );

    assert(tab != nullptr);
    for ( size_t index = 0; index < size_tab; index++){
        tab[index] = index;
    }
    for ( size_t index = 0; index < size_tab; index++){
        assert(tab[index] == index);
    }

    int *tab2 = (int*)heap_calloc(sizeof(int), size_tab);
    for ( size_t index = 0; index < size_tab; index++){
        assert(tab2[index] == 0 );
        tab2[index] = index;
    }
    for ( size_t index = 0; index < size_tab; index++){
        assert(tab2[index] == index);
    }
    assert( get_pointer_type(tab) == pointer_valid );
    assert( get_pointer_type(tab2) == pointer_valid );

    heap_free(tab);
    heap_free(tab2);

    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

//     już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
}

void test3(){
    int status = heap_setup();
    assert(status == 0);
    int const size_tab = 20;
    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();
    char *_string = (char *)heap_malloc(sizeof(char)*size_tab);
    assert( get_pointer_type(_string) == pointer_valid );


    for ( size_t i = 0; i < size_tab; i++){
        _string[i] = 'q';
    }
    for ( size_t i = 0; i < size_tab; i++){
        assert(_string[i] == 'q');
    }
    assert( get_pointer_type(_string) == pointer_valid );
    auto temp_realoc = (char*)heap_realloc(_string, 2 * size_tab);
    assert( get_pointer_type(temp_realoc) == pointer_valid );

    assert(temp_realoc != nullptr);
    _string = temp_realoc;
    temp_realoc = nullptr;
    assert(pointer_valid == get_pointer_type(_string));
    for ( size_t i = 0; i < size_tab; i++){
        assert(_string[i] == 'q');
    }
    for ( size_t i = size_tab; i < 2*size_tab; i++){
        _string[i] = 'a';
    }
    for ( size_t i = size_tab; i < 2*size_tab; i++){
        assert(_string[i] == 'a');
    }

    heap_free(_string);

    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

//     już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
}

void test4(){
    int status = heap_setup();
    assert(status == 0);
    int const size_tab = 20;
    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void *p1 = heap_malloc(3 * 1024);
    assert(p1 != nullptr);
    void *p2 = heap_malloc(3 * 1024 * 1024);
    assert(p2 != nullptr);

    void *p3 = heap_malloc(3 * 1024);
    assert(p3 != nullptr);
    heap_free(p2);

    void *p4 = heap_malloc(1 * 1024 * 1024);
    assert(p4 != nullptr);
    heap_free(p1);
    void *p5 = heap_malloc(2 * 1024 * 1024);
    assert(p5 != nullptr);
//    heap_free(p1);
    void *p6 = heap_malloc(3 * 1024);
    assert(p6 != nullptr);
    heap_free(p5);

    void *p7 = heap_malloc(1024);
    assert(p7 != nullptr);

    void *p8 = heap_malloc(100);
    assert(p8 != nullptr);
    void *p9 = heap_malloc(50);
    assert(p9 != nullptr);
    heap_free(p8);
    void *p10 = heap_malloc(10);
    assert(p10 != nullptr);
    heap_free(p9);
    heap_free(p3);
    heap_free(p4);
    heap_free(p6);
    heap_free(p7);
    heap_free(p10);
    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

//     już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
}

void test5()
{
    int status = heap_setup();
    assert(status == 0);
    int const size_tab = 1024;
    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    int *tab = (int*)heap_malloc(size_tab * 4);
    assert( tab != nullptr );
    assert( pointer_valid == get_pointer_type(tab) );
    for ( size_t i = 0; i < size_tab; i++){
        tab[i] = rand() % size_tab;
    }
    heap_free(tab);
    int *tab1 = (int*)heap_calloc(size_tab,  4);
    assert( tab1 != nullptr && pointer_valid == get_pointer_type(tab1) );
    for ( size_t i = 0; i < size_tab; i++){
        assert(tab1[i] == 0 );
    }
    heap_free(tab1);

    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

//     już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
}

void  test6()
{
    int status = heap_setup();
    assert(status == 0);
    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    auto p = heap_malloc_aligned(123);
    auto k = heap_malloc(123*1024);

    auto p1 = heap_malloc_aligned(123);
    auto k1 = heap_malloc(123*3);

    auto p2 = heap_malloc_aligned(1024);
    auto k2 = heap_malloc(2*1024*1024);

//    auto p3 = heap_malloc_aligned(123);
    heap_dump_debug_information();
     heap_free(p);
     heap_free(k);
    heap_free(p1);
    heap_free(k1);
    heap_free(p2);
    heap_free(k2);
//    heap_free(p3);
    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);
    heap_validate();
//     już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);
//    heap_dump_debug_information();
}