//
// Created by 抑~风 on 2023-03-04.
//

#include <exception>
#include <iostream>
#include <new>
#include "jemalloc/jemalloc.h"
#include "newoperator.h"

#ifdef CWJ_JEMALLOC_OPEN

void *operator new(size_t size){
//    std::cout<<"====== new ========"<<std::endl;
    auto ret = je_malloc(size);
    if(!ret) (throw std::bad_alloc());
    return ret;
}

void *operator new[](size_t size) {
//    std::cout<<"====== new[] ========"<<std::endl;
    auto ret = je_malloc(size);
    if(!ret) throw std::bad_alloc();
    return ret;
}

void operator delete (void *v) noexcept{
//    std::cout<<"====== delete ========"<<std::endl;
    je_free(v);
}

void operator delete [](void *v) noexcept{
//    std::cout<<"====== delete[] ========"<<std::endl;
    je_free(v);
}

#endif