//
// Created by 抑~风 on 2023/2/4.
//

#ifndef CWJ_CO_NET_TIMER_H
#define CWJ_CO_NET_TIMER_H

#include <inttypes.h>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "mutex.h"

namespace CWJ_CO_NET {

    class TimerManager;

    class Timer : public std::enable_shared_from_this<Timer> {
    public:
        using ptr = std::shared_ptr<Timer>;
        using CallBack = std::function<void()>;

        Timer(uint64_t mMs, const CallBack &mCb, bool mRecurring);

        bool refresh();

        bool reset(uint64_t ms, bool from_now);

        bool cancel();

        TimerManager *getMManager() const;

        void setMManager(TimerManager *mManager);

        uint64_t getMNext() const;

        const CallBack &getMCb() const;

        bool isMRecurring() const;



    private:
        uint64_t m_ms = 0;
        uint64_t m_next = 0;
        CallBack m_cb = nullptr;
        TimerManager* m_manager = nullptr;
        bool m_recurring = false;

    public:
        struct Comparer{
            bool operator()(const Timer::ptr& lhs,const Timer::ptr &rhs){
                if(lhs == rhs)  return false;
                if(!lhs)    return true;
                if(!rhs)    return false;
                return lhs->m_next < rhs->m_next;
            }
        };
    };


    class TimerManager {

    public:
        using MutexType = RWMutex;
        using CallBack = Timer::CallBack;
        using ptr = std::shared_ptr<TimerManager>;

        friend class Timer;

        void addTimer(uint64_t ms, CallBack cb, bool recurring = false);

        void addConditionTimer(uint64_t ms, CallBack cb, std::weak_ptr<void> con, bool recurring);

        void addTimerNotLock(Timer::ptr timer);

        uint64_t getNextTimer();

        void listExpiredCb(std::vector<CallBack> &list);

        bool hasTimer();

    protected:

        // 注意：该函数实现不能加m_mutex，否则很可能造成死锁
        virtual void onTimerInsertedAtFront() = 0;

    private:
        MutexType m_mutex;
        std::set<Timer::ptr,Timer::Comparer> m_timers;
    };

}


#endif //CWJ_CO_NET_TIMER_H
