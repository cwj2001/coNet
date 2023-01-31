//
// Created by 抑~风 on 2023/1/30.
//
#include <cstddef>
#include <ucontext.h>
#include <atomic>

#include "coroutine.h"
#include "macro.h"
#include "log.h"
#include "allocator.h"
#include "config.h"
#include "scheduler.h"

namespace CWJ_CO_NET {

    static Logger::ptr g_logger = GET_LOGGER("system");
    static auto g_stack_size = ConfigVar<int>::ptr(new ConfigVar<int>("", "", 128 *
                                                                              1024));// GET_CONFIG_MGR()->lookup<int>("cwj_co_net.coroutine.stack_size",64*1024,"coroutine stack size");

    static thread_local Coroutine::ptr g_thread_main_co;
    static thread_local Coroutine::ptr g_thread_cur_co;
    std::atomic<size_t> g_co_id{1};
    std::atomic<size_t> g_co_count{0};

    Coroutine::Coroutine() {
        m_id = g_co_id;
        g_co_id++;
        m_state = CoState::EXEC;
        std::cout << "";
        if (getcontext(&m_ctx)) {
            ERROR_LOG(g_logger) << "getcontext error errno :" << errno;
            CWJ_ASSERT_MGS(false, "getcontext error");
            assert(false);
        }
        g_co_count++;
    }

    Coroutine::Coroutine(Coroutine::CallBack cb, size_t stack_size, bool use_scheduler) : m_use_scheduler(
            use_scheduler) {

        CWJ_ASSERT(GetThis());// 不仅仅是断言，还必须执行GetThis，因为该函数有副作用

        m_id = (g_co_id++);

        m_state = CoState::INIT;
        m_cb = cb;

        m_stack_size = stack_size ? stack_size : g_stack_size->getMVal();

        m_stack = Allocator::Alloc(m_stack_size);

        if (getcontext(&m_ctx)) {
            CWJ_ASSERT_MGS(false, "Coroutine::Coroutine(Coroutine::CallBack, size_t, bool)  getcontext fail");
        }

        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stack_size;
        m_ctx.uc_stack.ss_flags = 0;

        makecontext(&m_ctx, &Coroutine::Run, 0);

        g_co_count++;

        INFO_LOG(g_logger) << "co :" << m_id << " create successfully ";

    }


    void Coroutine::call() {
        if (m_state == CoState::TERM || m_state == CoState::EXCEPT) return;
        if (g_thread_cur_co == g_thread_main_co) return;

//        INFO_LOG(g_logger) << "Coroutine::call ";
        g_thread_cur_co = this->shared_from_this();
        m_state = CoState::EXEC;

        if (m_use_scheduler) {

            auto co = Scheduler::GetScheduleCo();
            CWJ_ASSERT(co);
            if (swapcontext(&co->m_ctx, &m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }

        } else {

            CWJ_ASSERT(g_thread_main_co);
            if (swapcontext(&g_thread_main_co->m_ctx, &m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }

        }
    }

    void Coroutine::back() {
        if (g_thread_cur_co == g_thread_main_co) return;
        g_thread_cur_co = g_thread_main_co;
        if(m_use_scheduler){
            auto co = Scheduler::GetScheduleCo();
            CWJ_ASSERT(co);
            if (swapcontext(&m_ctx, &co->m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }
        }else {
            CWJ_ASSERT(g_thread_main_co);
            if (swapcontext(&m_ctx, &g_thread_main_co->m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }
        }
    }

    Coroutine::ptr Coroutine::GetThis() {

        if (!g_thread_cur_co) {
            auto main_co = Coroutine::ptr(new Coroutine);
            CWJ_ASSERT_MGS(main_co, "Coroutine::GetThis() create main co fails");
            g_thread_main_co = main_co;
            g_thread_cur_co = main_co;
        }
        return g_thread_cur_co;
    }

    void Coroutine::Run() {

        auto cur_co = GetThis();
        CWJ_ASSERT(cur_co);

        try {
            cur_co->m_cb();
            INFO_LOG(g_logger) << "m_cb finish";
            cur_co->m_state = CoState::TERM;
        } catch (std::exception &e) {
            cur_co->m_state = CoState::EXCEPT;
            ERROR_LOG(g_logger) << "Coroutine::run() fail" << BacktraceToStr();
        }

        INFO_LOG(g_logger) << "can reash run finish";

        auto p = cur_co.get();
        cur_co.reset();

        p->back();

        CWJ_ASSERT_MGS(false, "cannot reach here id :" + std::to_string(GetId()));

    }

    size_t Coroutine::GetId() {
        if (g_thread_cur_co) return g_thread_cur_co->m_id;
        return 0;
    }

    void Coroutine::YieldToReady() {

        INFO_LOG(g_logger) << "Coroutine::YieldToReady()";

        auto cur_co = GetThis();
        if (cur_co == g_thread_main_co) return;
        cur_co->m_state = CoState::READY;
        cur_co->back();
    }

    size_t Coroutine::getMId() const {
        return m_id;
    }

    Coroutine::~Coroutine() {

        if (m_stack) {

            CWJ_ASSERT(m_state == CoState::EXCEPT || m_state == CoState::TERM || m_state == CoState::INIT);
            Allocator::Dealloc(m_stack, m_stack_size);
        } else {
            // 原始的主协程
            CWJ_ASSERT(m_state == CoState::EXEC);

        }
        INFO_LOG(g_logger) << "Coroutine::~Coroutine() ";
    }
}
