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
    static auto g_stack_size = ConfigVar<int>::ptr(new ConfigVar<int>("", "", 128*1024));// GET_CONFIG_MGR()->lookup<int>("cwj_co_net.coroutine.stack_size",64*1024,"coroutine stack size");

    static thread_local Coroutine::ptr g_thread_main_co;
    static thread_local Coroutine::ptr g_thread_cur_co;
    std::atomic<size_t> g_co_id{1};
    std::atomic<size_t> g_co_count{0};

    Coroutine::Coroutine() : m_use_scheduler(false) {
        m_id = g_co_id;
        g_co_id++;
        m_state = CoState::EXEC;
#ifdef USE_UCONTEXT
        if (getcontext(&m_ctx)) {
            ERROR_LOG(g_logger) << "getcontext error errno :" << errno;
            CWJ_ASSERT_MGS(false, "getcontext error");
            assert(false);
        }
#endif
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
#ifdef USE_UCONTEXT
        if (getcontext(&m_ctx)) {
            CWJ_ASSERT_MGS(false, "Coroutine::Coroutine(Coroutine::CallBack, size_t, bool)  getcontext fail");
        }

        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stack_size;
        m_ctx.uc_stack.ss_flags = 0;

        makecontext(&m_ctx, &Coroutine::Run, 0);

#else



        char **stack = (char **)((char*)m_stack + m_stack_size);

        stack[-3] = NULL;
        stack[-2] = NULL;

        m_cpu_ctx.esp = (char*)stack - (2 * sizeof(void*));
        m_cpu_ctx.ebp = (char*)stack - (1 * sizeof(void*));
        m_cpu_ctx.eip = (void*)&Coroutine::Run;

#endif

        g_co_count++;

        INFO_LOG(g_logger) << "co :" << m_id << " create successfully ";

    }

    void  Coroutine::reset(CallBack cb,size_t stack_size,bool use_scheduler){

        // 处理上下文
        m_state = CoState::INIT;
        m_cb = cb;
#ifdef USE_UCONTEXT
        if (getcontext(&m_ctx)) {
            CWJ_ASSERT_MGS(false, "Coroutine::Coroutine(Coroutine::CallBack, size_t, bool)  getcontext fail");
        }
#endif

        // 处理stack

        if(stack_size > 0 && (stack_size > m_stack_size)){
            ERROR_LOG(g_logger) << "alloc stack ";
            Allocator::Dealloc(m_stack,m_stack_size);
            m_stack_size = stack_size;
            m_stack = Allocator::Alloc(m_stack_size);
        }
#ifdef USE_UCONTEXT
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stack_size;
        m_ctx.uc_stack.ss_flags = 0;

        makecontext(&m_ctx, &Coroutine::Run, 0);
        INFO_LOG(g_logger) << "co :" << m_id << " reused ";
#else

        void **stack = (void **)((char*)m_stack + m_stack_size);
        m_cpu_ctx.esp = (char*)stack - (4 * sizeof(void*));
        m_cpu_ctx.ebp = (char*)stack - (3 * sizeof(void*));
        m_cpu_ctx.eip = (void*)&Coroutine::Run;

#endif
        // 处理use

        m_use_scheduler = use_scheduler;

    }


    void Coroutine::call() {

        if (m_state == CoState::TERM || m_state == CoState::EXCEPT) return;
        if (g_thread_cur_co == shared_from_this()) return;

        g_thread_cur_co = this->shared_from_this();
        m_state = CoState::EXEC;
        if (m_use_scheduler) {
            auto co = Scheduler::GetScheduleCo();
            CWJ_ASSERT(co);

#ifdef USE_UCONTEXT
            if (swapcontext(&co->m_ctx, &m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }
#else

            swapCpuCtx(&m_cpu_ctx,&co->m_cpu_ctx);

#endif
        } else {

            CWJ_ASSERT(g_thread_main_co);

#ifdef USE_UCONTEXT

            if (swapcontext(&g_thread_main_co->m_ctx, &m_ctx)) {
                CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
            }
#else

            swapCpuCtx(&m_cpu_ctx,&g_thread_main_co->m_cpu_ctx);

#endif

        }
    }

    void Coroutine::back() {
        Coroutine::ptr co = nullptr;
        co = GetMainCo(shared_from_this());

        swapCoroutine(co);
    }

    Coroutine::ptr Coroutine::GetMainCo(Coroutine::ptr co) {

        if(co->m_use_scheduler){
            return Scheduler::GetScheduleCo();
        }else {
            return g_thread_main_co;
        }
    }

    void Coroutine::swapCoroutine(const Coroutine::ptr &co){
        CWJ_ASSERT(co);
        if(g_thread_cur_co == co)   return ;
        g_thread_cur_co = co;
//        CWJ_ASSERT(false);
        co->m_state = CoState::EXEC;

#ifdef USE_UCONTEXT

        if (swapcontext(&m_ctx, &co->m_ctx)) {
            CWJ_ASSERT_MGS(false, "Coroutine::call swapcontext fail");
        }

#else

        DEBUG_LOG(g_logger) << "Coroutine:: swap";
        swapCpuCtx(&co->m_cpu_ctx,&m_cpu_ctx);

#endif

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
            cur_co->m_state = CoState::TERM;
        } catch (std::exception &e) {
            cur_co->m_state = CoState::EXCEPT;
        }

        auto p = cur_co.get();
        cur_co.reset();
//        CWJ_ASSERT(false);
        p->back();

        CWJ_ASSERT_MGS(false, "cannot reach here id :" + std::to_string(GetId()));

    }

    size_t Coroutine::GetId() {
        if (g_thread_cur_co) return g_thread_cur_co->m_id;
        return 0;
    }

    void Coroutine::YieldToReady() {

        DEBUG_LOG(g_logger) << "Coroutine::YieldToReady()";
        auto cur_co = GetThis();
        auto main_co = GetMainCo(cur_co);
        if (cur_co == main_co) return;

        cur_co->m_state = CoState::READY;

        cur_co->back();
    }

    size_t Coroutine::getMId() const {
        return m_id;
    }

    Coroutine::~Coroutine() {

//        CWJ_ASSERT(m_use_scheduler);
        if (m_stack) {
            CWJ_ASSERT(m_state == CoState::EXCEPT || m_state == CoState::TERM || m_state == CoState::INIT);
            Allocator::Dealloc(m_stack, m_stack_size);
        } else {
            // 原始的主协程
            CWJ_ASSERT(m_state == CoState::EXEC);

        }
//        INFO_LOG(g_logger) << "Coroutine::~Coroutine() ";
    }

    void Coroutine::YieldToHold() {



        auto cur_co = GetThis();
        if (cur_co == g_thread_main_co) return;
        cur_co->m_state = CoState::HOLD;
        cur_co->back();

    }

    CoState::State Coroutine::getMState() const {
        return m_state;
    }

    void Coroutine::setMState(CoState::State mState) {
        m_state = mState;
    }
}
