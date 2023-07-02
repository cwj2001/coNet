//
// Created by 抑~风 on 2023/1/29.
//

#ifndef CWJ_CO_NET_NONCOPYABLE_H
#define CWJ_CO_NET_NONCOPYABLE_H

namespace CWJ_CO_NET{

    class NonCopyAble{

    public:

        NonCopyAble() = default;

        NonCopyAble(const NonCopyAble&) = delete;

        virtual ~NonCopyAble() = default;

        NonCopyAble& operator=(const NonCopyAble&) = delete;

    };

}

#endif //CWJ_CO_NET_NONCOPYABLE_H
