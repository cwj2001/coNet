#include <stdio.h>
#include <stdlib.h>
#include "jemalloc/jemalloc.h"
#include "newoperator.h"
int main() {
    void *ptr = malloc(1024);
    printf("%p\n", ptr);
    free(ptr);

    void *ptr2 = malloc(1024);
    printf("%p\n", ptr2);
    free(ptr2);

    auto t = new int();
    delete t;

    return 0;
}
