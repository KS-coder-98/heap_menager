//
// Created by root on 1/2/20.
//

#include <cstdio>
#include <cstring>
#include "custom_unistd.h"

void *return_set_debug(int fileline, const char *filename, memblock_t *temp);

int heap_setup()
{
    heap_menager_.lock.lock();
    assert(!heap_menager_.init);

    auto *start_block = (memblock_t *)custom_sbrk(0);
    start_block->init_memblock();
    if ( custom_sbrk(PAGE_SIZE) == (void*)-1 ){ // tod zmienic sporwrotem na PAGE size
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

    heap_menager_.lock.unlock();
    return 0;
}

void heap_dump_debug_information()
{
    heap_menager_.lock.lock();
    if ( heap_menager_.heap_head == nullptr ){
        heap_menager_.lock.unlock();
        return;
    }
    printf("Size mataData %lu\n", SIZE_METADANE);
    for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next) {
        printf("\n##############################\n");
        printf("\tAddress %p\n", iterator);
        printf("\tData address %p\n", iterator->data);
        printf("\tSize %zu\n", iterator->size);
        printf("\tchceck sum %zu\n", iterator->checksum);
        if ( iterator->line != -1 ){
            printf("\tLine %d\n", iterator->line);
            printf("\tFile %s\n", iterator->name_file);
        }
        if ( iterator->status_ == status::FREE )
            printf("\tIs free Yes\n");
        else if ( iterator->status_ == status::NOT_FREE )
            printf("\tIs free No\n");
        else
            printf("\tundefitned\n");

        printf("##############################\n");
    }
    heap_menager_.lock.unlock();
}

void memblock_t::init_memblock() {
    heap_menager_.lock.lock();
    for ( size_t i = 0; i<4; i++){
        fence_start[i] = 9;
        fence_end[i] = 9;
    }
    data = this + 1;
    status_ = status::NOT_FREE;
    line = -1;
    size_t temp1=0;
    for (int i : fence_start)
        temp1+=i;
    for (int i : fence_end)
        temp1 +=i;
    checksum = temp1;
    heap_menager_.lock.unlock();
}

void* heap_malloc(size_t count)
{
    heap_menager_.lock.lock();
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
            int error = extend_heap(count);
            if ( error == -1 ){
                heap_menager_.lock.unlock();
                return nullptr;
            }
            else if ( error == 0 ){
                heap_menager_.lock.unlock();
                return heap_malloc(count);
            }
        }
    }
    else{
        heap_setup();
        heap_menager_.lock.unlock();
        return heap_malloc(s);
    }
    assert(new_block!= nullptr);
    new_block->data = new_block+1;
    return ( new_block->data );
};
void* heap_calloc(size_t number, size_t size)
{
    heap_menager_.lock.lock();
    auto temp1 = (memblock_t*)heap_malloc(number*size);
    memset(temp1, 0, number*size);
    heap_menager_.lock.unlock();
    return temp1;
};
void heap_free(void* block)
{
    heap_menager_.lock.lock();
    assert(get_pointer_type(block) == pointer_valid);
    auto temp = (memblock_t*)block - 1;
    temp->status_= status::FREE;
    fusion(temp);
    heap_menager_.lock.unlock();
    reduce_heap();
};
void* heap_realloc(void* block, size_t size)
{
    heap_menager_.lock.lock();
    heap_validate();
    if ( !block ) {
        heap_menager_.lock.unlock();
        return heap_malloc(size);
    }
    assert( get_pointer_type(block) == pointer_valid );
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
            auto new_block = (memblock_t*)heap_malloc(size);
            if ( !new_block ){
                heap_menager_.lock.unlock();
                return nullptr;
            }
            new_block->init_memblock();
            temp_block->status_ = status::FREE;
            fusion(temp_block);
            heap_menager_.lock.unlock();
            return memcpy(new_block->data, temp_block->data, temp_block->size);
        }
    }
    heap_menager_.lock.unlock();
    return temp_block->data;
};

memblock_t* find_block(size_t size)
{
    heap_menager_.lock.lock();
    auto temp_block = heap_menager_.heap_head;
    while ( temp_block ){
        if ( temp_block->size >= size + SIZE_METADANE  && temp_block->status_ == status::FREE){
            heap_menager_.lock.unlock();
            return temp_block;
        }
        temp_block = temp_block->next;
    }
    heap_menager_.lock.unlock();
    return nullptr;
}

memblock_t* find_block_aligned(size_t size)
{
    heap_menager_.lock.lock();
    auto temp_block = heap_menager_.heap_head;
    while ( temp_block ){
        if ( temp_block->size >= size + SIZE_METADANE  && temp_block->status_ == status::FREE && (((intptr_t)temp_block & (intptr_t)(PAGE_SIZE - 1)) == 0) ){
            heap_menager_.lock.unlock();
            return temp_block;
        }
        temp_block = temp_block->next;
    }
    heap_menager_.lock.unlock();
    return nullptr;
}

memblock_t* find_block_aligned_in_free_space(size_t size)
{
    heap_menager_.lock.lock();
    auto temp_block = heap_menager_.heap_head;
    while ( temp_block ){
        if ( temp_block->status_ == status::FREE ){
            auto tempPtr = (intptr_t)heap_menager_.heap_head;
            assert(tempPtr);
            while ( tempPtr < (intptr_t)temp_block->next){
                tempPtr += PAGE_SIZE;
            }
            tempPtr-= PAGE_SIZE;
            auto new_algined_block = (memblock_t*) tempPtr;
            auto size_from_new_page = temp_block->size-(tempPtr - (intptr_t)temp_block->data);
            if (  size_from_new_page >= size  && new_algined_block >= temp_block->data){ //wystarczajaco duzo mjejsca w wolnym bloku od wielokrotnosci strony
                //stworzyc nowy blok
                new_algined_block -=1;
                new_algined_block->init_memblock();

                new_algined_block->prev = temp_block;
                new_algined_block->next = new_algined_block->prev->next;

                new_algined_block->next->prev = new_algined_block;
                new_algined_block->prev->next = new_algined_block;

                new_algined_block->size = size_from_new_page;

                temp_block->size = (intptr_t)(new_algined_block)-(intptr_t)temp_block->data; // temp_block wolny blok o wystarczajaco duzej przestrzeni

                split_block((memblock_t*)new_algined_block, size);
                heap_menager_.lock.unlock();
                return new_algined_block;
            }
            assert(((intptr_t)tempPtr & (intptr_t)(PAGE_SIZE - 1)) == 0);
        }
        temp_block = temp_block->next;
    }
    heap_menager_.lock.unlock();
    return nullptr;
}



void split_block(memblock_t* block, size_t s)
{
    heap_menager_.lock.lock();
    memblock_t* new_block;
    assert(block->data != nullptr);
    new_block = (memblock_t*)((intptr_t)block->data + s);
    new_block->init_memblock();
    new_block->size = block->size - s - SIZE_METADANE; //zmienic
    new_block->next = block->next;
    new_block->status_ = status::FREE;
    block->size = s;
    block->next = new_block;
    new_block->prev = block;
    new_block->status_ = status::FREE;
    new_block->next->prev = new_block;
    heap_menager_.lock.unlock();
}

memblock_t* fusion(memblock_t* block)
{
    heap_menager_.lock.lock();
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
    heap_menager_.lock.unlock();
    return block;
}

int extend_heap(size_t counter)
{
    heap_menager_.lock.lock();
    int number_of_pages_neeeded;
    number_of_pages_neeeded = ceil((double)counter/(double)PAGE_SIZE);
    auto old_end = (memblock_t*)custom_sbrk(0) - 1;
    if ( custom_sbrk(number_of_pages_neeeded * PAGE_SIZE) == (void*)-1 ){
        /* sbrk fail*/
        heap_menager_.lock.unlock();
        return -1;
    }
    auto new_end_block = (memblock_t*)custom_sbrk(0) - 1;
    new_end_block->init_memblock();
    new_end_block->next = nullptr;
    new_end_block->prev = old_end;

    old_end->next = new_end_block;
    old_end->status_ = status::FREE;
    old_end->size += PAGE_SIZE * number_of_pages_neeeded - SIZE_METADANE;
    //tu jest blad ze wskzniki bloku old end wskzuje na poczatek pierwszej strony !!!!
    fusion(old_end); //chceck

    heap_menager_.heap_tail = new_end_block;
    heap_menager_.lock.unlock();
    return 0;
}

void test_linked_list()
{
    heap_menager_.lock.lock();
    assert(heap_menager_.heap_tail != nullptr);
    assert(heap_menager_.heap_head != nullptr);
    for ( auto ptr = heap_menager_.heap_head; ptr != heap_menager_.heap_tail; ptr = ptr->next )
    {
        if ( ptr != heap_menager_.heap_head ){
            assert(ptr == ptr->prev->next);
        }
        if ( ptr != heap_menager_.heap_tail ){
            assert(ptr == ptr->next->prev);
//            assert(1!=1); //to zatrzyma program
        }
        assert(ptr->checksum == ptr->count_checksum());
        for ( size_t value : ptr->fence_start )
            assert(value == 9);
        for ( size_t value : ptr->fence_end )
            assert(value == 9);
    }
    heap_menager_.lock.unlock();
}

void *heap_malloc_debug(size_t count, int fileline, const char *filename) {
    heap_menager_.lock.lock();
    auto temp = (memblock_t*)heap_malloc(count)-1;
    heap_menager_.lock.unlock();
    return return_set_debug(fileline, filename, temp);
}

void *return_set_debug(int fileline, const char *filename, memblock_t *temp) {
    heap_menager_.lock.lock();
    if (temp == nullptr ){
        heap_menager_.lock.unlock();
        return nullptr;
    }
    temp->line = fileline;
    temp->name_file = (char*)filename;
    heap_menager_.lock.unlock();
    return temp->data;
}

void *heap_calloc_debug(size_t number, size_t size, int fileline, const char *filename) {
    heap_menager_.lock.lock();
    auto temp = (memblock_t*)heap_calloc(number ,size);
    heap_menager_.lock.unlock();
    return return_set_debug(fileline, filename, temp);
}

void *heap_realloc_debug(void *memblock, size_t size, int fileline, const char *filename) {
    heap_menager_.lock.lock();
    auto temp = (memblock_t*)heap_realloc(memblock, size)-1;
    heap_menager_.lock.unlock();
    return return_set_debug(fileline, filename, temp);
}

int heap_validate(void)
{
    heap_menager_.lock.lock();
    test_linked_list();
    assert(heap_menager_.heap_tail != nullptr);
    assert(heap_menager_.heap_head != nullptr);
    for ( auto ptr = heap_menager_.heap_head; ptr != heap_menager_.heap_tail; ptr = ptr->next )
    {
        if ( ptr != heap_menager_.heap_head ){
            assert(ptr == ptr->prev->next);
        }
        if ( ptr != heap_menager_.heap_tail ){
            assert(ptr == ptr->next->prev);
            auto t1 = ((intptr_t)ptr->data + ptr->size);
            auto t2 = (intptr_t)ptr->next;
            printf("%zu %zu %lu\n", t1, t2, t2-t1);
            assert( ((intptr_t)ptr->data + ptr->size) == (intptr_t)ptr->next);

//            assert(1!=1); //to zatrzyma program
        }
        assert(ptr + 1 == ptr->data);
        assert(ptr->status_==status::FREE || ptr->status_==status::NOT_FREE);
    }
    heap_menager_.lock.unlock();
    return 0;
}

size_t memblock_t::count_checksum()
{
    heap_menager_.lock.lock();
    size_t temp = 0;
    for (int i : fence_start)
        temp += i;
    for (int i : fence_end)
        temp += i;
    heap_menager_.lock.unlock();
    return temp;
}

size_t memblock_t::set_checksum()
{
    checksum = count_checksum();
    return checksum;
}

size_t heap_get_used_space(void)
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head; ptr; ptr=ptr->next ){
        if ( ptr->status_ == status::FREE )
            result+=SIZE_METADANE;
        else
            result+=(SIZE_METADANE+ptr->size);
    }
    heap_menager_.lock.unlock();
    return result;
}

size_t heap_get_largest_used_block_size()
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head; ptr; ptr=ptr->next ){
        if ( ptr->status_ == status::NOT_FREE )
            result = result < ptr->size ? ptr->size : result;
    }
    heap_menager_.lock.unlock();
    return result;
}

uint64_t heap_get_used_blocks_count(void)
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head->next; ptr != heap_menager_.heap_tail; ptr=ptr->next ){
        if ( ptr->status_ == status::NOT_FREE)
            result++;
    }
    heap_menager_.lock.unlock();
    return result;
}

size_t heap_get_free_space(void)
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head; ptr; ptr=ptr->next ){
        result+=(SIZE_METADANE+ptr->size);
    }
    result -= heap_get_used_space();
    heap_menager_.lock.unlock();
    return result;
}

size_t heap_get_largest_free_area(void)
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head; ptr; ptr=ptr->next ){
        if ( ptr->status_ == status::FREE )
            result = result < ptr->size ? ptr->size : result;
    }
    heap_menager_.lock.unlock();
    return result;
}

uint64_t heap_get_free_gaps_count(void)
{
    heap_menager_.lock.lock();
    size_t result = 0;
    for ( auto ptr = heap_menager_.heap_head; ptr; ptr=ptr->next ){
        if ( ptr->status_ == status::FREE && ptr->size >= sizeof(void*))
            result++;
    }
    heap_menager_.lock.unlock();
    return result;
}

enum pointer_type_t get_pointer_type(const void* pointer)
{
    heap_menager_.lock.lock();
    if ( !pointer ) {
        heap_menager_.lock.unlock();
        return pointer_null;
    }
    else if ( pointer < heap_menager_.heap_head || pointer > heap_menager_.heap_tail ){
        heap_menager_.lock.unlock();
        return pointer_out_of_heap;
    }
    else{
        for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next){
            if ( iterator->data == pointer && iterator->size != 0){
                heap_menager_.lock.unlock();
                return pointer_valid;
            }
            else if ( pointer >= iterator && pointer < iterator->data ){
                heap_menager_.lock.unlock();
                return pointer_control_block;
            }
            else if ( (char*)iterator->data + 1 >= pointer && pointer < iterator->next && iterator->status_ == status::NOT_FREE){
                heap_menager_.lock.unlock();
                return pointer_inside_data_block;
            }
            else if ( (char*)iterator->data + 1 >= pointer && pointer < iterator->next && iterator->status_ == status::FREE){
                heap_menager_.lock.unlock();
                return pointer_unallocated;
            }
        }
    }

}

memblock_t* getHead()
{
    heap_menager_.lock.lock();
    auto temp = heap_menager_.heap_head;
    heap_menager_.lock.unlock();
    return temp;
}

bool reduce_heap() {
    heap_menager_.lock.lock();
    auto temp_end = heap_menager_.heap_tail;
    if ( temp_end->prev->status_ == status::FREE ){ // try reduce
        auto temp_free_last_block = temp_end->prev;
        auto number_size_sub = temp_free_last_block->size/PAGE_SIZE;
        if ( number_size_sub > 0 ){
            custom_sbrk(-number_size_sub*PAGE_SIZE);

            auto end_block = (memblock_t*)custom_sbrk(0) - 1;
            end_block->init_memblock();
            end_block->status_ = status::NOT_FREE;
            end_block->size = 0;
            end_block->next = nullptr;

            //connecting block
            end_block->prev = temp_free_last_block;
            temp_free_last_block->next = end_block;
            temp_free_last_block->size -= (PAGE_SIZE * number_size_sub);

            heap_menager_.heap_tail = end_block;
            heap_menager_.lock.unlock();
            return  true;
        }
    }
    heap_menager_.lock.unlock();
    return false;
}

void* heap_get_data_block_start(const void* pointer) // przetestowac
{
    heap_menager_.lock.lock();
    auto type_block = get_pointer_type(pointer);
    if ( type_block == pointer_null || type_block == pointer_out_of_heap ) {
        heap_menager_.lock.unlock();
        return nullptr;
    }
    else if ( type_block == pointer_valid ) {
        heap_menager_.lock.unlock();
        return (void *) pointer;
    }
    for (auto iterator = heap_menager_.heap_head; iterator; iterator = iterator->next){
        if ( iterator->data == pointer && iterator->size != 0){
            heap_menager_.lock.unlock();
            return iterator->data;
        }
        else if ( pointer >= iterator && pointer < iterator->data ){
            heap_menager_.lock.unlock();
            return iterator->data;
        }
        else if ( (char*)iterator->data + 1 >= pointer && pointer < iterator->next && iterator->status_ == status::NOT_FREE){
            heap_menager_.lock.unlock();
            return iterator->data;
        }
        else if ( (char*)iterator->data + 1 >= pointer && pointer < iterator->next && iterator->status_ == status::FREE){
            heap_menager_.lock.unlock();
            return iterator->data;
        }
    }
}

size_t heap_get_block_size(const void* memblock) // przetestowac
{
    heap_menager_.lock.lock();
    auto temp = (memblock_t*)memblock;
    if ( pointer_valid == get_pointer_type(temp) ){
        return (temp-1)->size;
    }
    heap_menager_.lock.unlock();
    return 0;
}

void* heap_malloc_aligned(size_t count)
{
    heap_menager_.lock.lock();
    memblock_t *new_block = nullptr;//, *last;
    size_t s;
    if (sizeof(void*) == 8 )
        s = align8(count);
    else
        s = align4(count);
    if ( heap_menager_.init ){
        new_block = find_block_aligned(s); // ok
        if ( new_block ){ //jesli znajdzie wystarczajaco duzy blok bedzie na krotnoscia strony - metadane size
            assert(((intptr_t)new_block & (intptr_t)(PAGE_SIZE - 1)) == 0);
            //czy mozna podzielic blok
            new_block->init_memblock();
            new_block->status_ = status::NOT_FREE;
            if ( (new_block->size - s) >= (SIZE_METADANE + sizeof(void*)) )
                split_block(new_block, s);
            new_block->status_ = status::NOT_FREE;
            heap_menager_.lock.unlock();
            return new_block->data;
        }
        else { // szukamy czy mozna uzyskać taki blok z wolnego mjesca
            new_block = find_block_aligned_in_free_space(s);
            if ( new_block != nullptr ){
                assert(((intptr_t)new_block->data & (intptr_t)(PAGE_SIZE - 1)) == 0);
                heap_menager_.lock.unlock();
                return new_block->data;
            }
        }
        //powiekszamy sterte
        int error = extend_heap(count);
        if ( error == -1 ){
            heap_menager_.lock.unlock();
            return nullptr;
        }
        else if ( error == 0 ){ // todo
            heap_menager_.lock.unlock();
            return heap_malloc_aligned(count);
        }
    }
    else{
        heap_setup();
        heap_menager_.lock.unlock();
        return heap_malloc_aligned(s);
    }
}
void* heap_calloc_aligned(size_t number, size_t size){
    heap_menager_.lock.lock();
    auto temp1 = (memblock_t*)heap_malloc_aligned(number*size);
    memset(temp1, 0, number*size);
    heap_menager_.lock.lock();
    return temp1;
}
void* heap_realloc_aligned(void* memblock, size_t size){ //todo
    heap_menager_.lock.lock();
    heap_validate();
    if ( !memblock ) {
        heap_menager_.lock.unlock();
        return heap_malloc_aligned(size);
    }
    assert( get_pointer_type(memblock) == pointer_valid );
    auto temp_block = (memblock_t*)memblock - 1;//todo sprawdzic to

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
            auto new_block = (memblock_t*)heap_malloc_aligned(size);
            if ( !new_block ) {
                heap_menager_.lock.unlock();
                return nullptr;
            }
            new_block->init_memblock();
            temp_block->status_ = status::FREE;
            fusion(temp_block);
            heap_menager_.lock.unlock();
            return memcpy(new_block->data, temp_block->data, temp_block->size);
        }
    }
    heap_menager_.lock.unlock();
    return temp_block->data;
}