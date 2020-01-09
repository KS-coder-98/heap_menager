#include <cstdio>
#include "custom_unistd.h"


int main(int argc, char **argv)
{
    // Tutaj można pisać kod
    // Zapoznaj się z funkcją main w bloku #if...#endif
    heap_setup();
//    int *test = (int *)heap_malloc(sizeof(int)*2);
    double *test2 = (double *)heap_malloc(sizeof(double)*2);
    double *test3 = (double *)heap_malloc(sizeof(double)*2);
//    test3[0] = 2.23;
//    test3[1]= 1.23;
//    heap_free(test3);
//    double *test6 = (double *)heap_malloc(2 * sizeof(double));

//    double *test6 = (double *)heap_calloc(2, sizeof(double));
//    printf("%lf %lf\n", test3[0], test3[1]);
//    double *test4 = (double *)heap_malloc(sizeof(double)*2);
//    double *test5 = (double *)heap_malloc(sizeof(double)*2);
//    heap_free(test);

    heap_free(test3);

    heap_free(test2);




//    heap_free(test4);
//    heap_free(test5);
    print_debug();
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