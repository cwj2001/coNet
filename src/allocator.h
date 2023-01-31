//
// Created by 抑~风 on 2023/1/31.
//

#ifndef CWJ_CO_NET_ALLOCATOR_H
#define CWJ_CO_NET_ALLOCATOR_H
#include <cstdlib>
namespace CWJ_CO_NET{
    class Allocator{
    public:
        static void * Alloc(size_t size) {
            return malloc(size);
        }

        static void Dealloc(void * buf,size_t size) {
            free(buf);
        }

    private:
    };
}

#endif //CWJ_CO_NET_ALLOCATOR_H
