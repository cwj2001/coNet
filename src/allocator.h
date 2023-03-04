//
// Created by 抑~风 on 2023/1/31.
//

#ifndef CWJ_CO_NET_ALLOCATOR_H
#define CWJ_CO_NET_ALLOCATOR_H
#include <cstdlib>
#include "jemalloc/jemalloc.h"
namespace CWJ_CO_NET{

#define CWJ_JEMALLOC_OPEN

    class Allocator{
    public:
        static void * Alloc(size_t size) {
#ifndef CWJ_JEMALLOC_OPEN
            return malloc(size);
#else
            return je_malloc(size);
#endif
        }

        static void Dealloc(void * buf,size_t size) {
#ifndef CWJ_JEMALLOC_OPEN
            return free(buf);
#else
            return je_free(buf);
#endif
        }

    private:
    };
}

#endif //CWJ_CO_NET_ALLOCATOR_H
