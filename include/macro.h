//
// Created by 抑~风 on 2023/1/30.
//

#ifndef CWJ_CO_NET_MACRO_H
#define CWJ_CO_NET_MACRO_H
#include "log.h"
#include "cassert"
#include "util.h"
namespace CWJ_CO_NET{

#define CWJ_ASSERT(x)                           \
    if(!(x)){                                   \
    FATAL_LOG(GET_LOGGER("system"))             \
    << "FALSE_ASSERT :"#x<<std::endl            \
    <<"Backtarce : \n"                          \
    <<CWJ_CO_NET::BacktraceToStr(256,0,"","\n");            \
    assert(false);                              \
}



#define CWJ_ASSERT_MGS(x,w)             \
    if(!(x)){                           \
    FATAL_LOG(GET_LOGGER("system"))     \
    << "FALSE_ASSERT :"#x<<std::endl    \
    << w << std::endl                   \
    <<"Backtarce : \n"                  \
    <<CWJ_CO_NET::BacktraceToStr(256,0,"","\n");    \
    assert(false);                      \
}


}

#endif //CWJ_CO_NET_MACRO_H
