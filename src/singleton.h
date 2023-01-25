//
// Created by 抑~风 on 2023/1/25.
//

#ifndef CWJ_CO_NET_SINGLETON_H
#define CWJ_CO_NET_SINGLETON_H

#include <memory>
namespace CWJ_CO_NET {
    template<typename T, typename X = void, int N = 0>
    class Singleton {
    public:
        static T *GetInstance() {
            static T one;
            return &one;
        }
    };


    template<typename T, typename X = void, int N = 0>
    class SingletonPtr {
    public:
        static std::shared_ptr<T> GetInstance() {
            static std::shared_ptr<T> one(new T);
            return one;
        }
    };
}

#endif //CWJ_CO_NET_SINGLETON_H
