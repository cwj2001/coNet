#include <stdio.h>
#include <stdlib.h>
#include "jemalloc/jemalloc.h"

int main() {
    void *ptr = malloc(1024);
    printf("%p\n", ptr);
    free(ptr);

    void *ptr2 = je_malloc(1024);
    printf("%p\n", ptr2);
    je_free(ptr2);

    return 0;
}
