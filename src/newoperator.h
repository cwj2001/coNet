//
// Created by 抑~风 on 2023-03-04.
//

#ifndef CWJ_CO_NET_NEWOPERATOR_H
#define CWJ_CO_NET_NEWOPERATOR_H

//#define CWJ_JEMALLOC_OPEN

#ifdef CWJ_JEMALLOC_OPEN

void *operator new(size_t);

void *operator new[](size_t) ;

void operator delete (void *) noexcept ;

void operator delete [](void *) noexcept;

#endif

#endif //CWJ_CO_NET_NEWOPERATOR_H
