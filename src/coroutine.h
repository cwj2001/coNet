//
// Created by 抑~风 on 2023/1/30.
//

#ifndef CWJ_CO_NET_COROUTINE_H
#define CWJ_CO_NET_COROUTINE_H

#include <memory>
#include <ucontext.h>
#include <functional>




namespace CWJ_CO_NET {

    class Scheduler;

    // 之所以不写成enum class 而写成类内类型的原因是，有可能到时有针对State的方法，可编译将处理State的方法设置为类的静态成员
    class CoState {
    public:
        enum State {
            INIT,
            READY,
            EXEC,
            HOLD,
            TERM,
            EXCEPT,
        };
    };

    class Coroutine : public std::enable_shared_from_this<Coroutine> {

    public:
        using ptr = std::shared_ptr<Coroutine>;
        using CallBack = std::function<void()>;

        friend class Scheduler;

        Coroutine(CallBack cb, size_t stack_size = 0, bool use_scheduler = false);

        virtual ~Coroutine();

        void call();

        void back();

        size_t getMId() const;



    private:

        Coroutine();

    public:

        static Coroutine::ptr GetThis();

        static size_t GetId();

        static void YieldToReady();

    private:

        static void Run(void);


    private:
        size_t m_id = 0;
        void *m_stack = nullptr;
        size_t m_stack_size = 0;

        ucontext_t m_ctx;

        CoState::State m_state = CoState::State::INIT;

        CallBack m_cb;

        bool m_use_scheduler;

    };

}
#endif //CWJ_CO_NET_COROUTINE_H
