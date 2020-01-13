#include <cstdio>
#include "custom_unistd.h"


int main(int argc, char **argv)
{
    // Tutaj można pisać kod
    // Zapoznaj się z funkcją main w bloku #if...#endif
    int *test1 = (int*)heap_calloc_debug(1, 4, __LINE__, __FILE__);
    int *test2 = (int*)heap_malloc_debug(40, __LINE__, __FILE__);
    int *test3 = (int*)heap_malloc(12* 4);
    int *test4 = (int*)heap_malloc(12);
    int *test11 = (int*)heap_calloc_debug(1, 4, __LINE__, __FILE__);
//    int *test21 = (int*)heap_malloc_debug(40, __LINE__, __FILE__);
//    int *test31 = (int*)(int*)heap_calloc_debug(10, 4, __LINE__, __FILE__);
//    int *test41 = (int*)(int*)heap_calloc_debug(10, 4, __LINE__, __FILE__);
    heap_free(test2);
    heap_free(test4);
//    heap_free(test1);

//    heap_setup();
    test_linked_list();
    heap_dump_debug_information();
    printf("\n\n%zu\n\n", heap_get_free_gaps_count());
    return 0;
}

#if 0
#include "custom_unistd.h"

int main(int argc, char **argv) {
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

	return 0;
}
#endif