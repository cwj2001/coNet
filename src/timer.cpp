//
// Created by 抑~风 on 2023/2/4.
//

#include "timer.h"
#include "util.h"
#include "macro.h"
#include "log.h"

namespace CWJ_CO_NET {

    static auto g_logger = GET_LOGGER("system");

    Timer::Timer(uint64_t mMs, const Timer::CallBack &mCb, bool mRecurring) : m_ms(mMs), m_cb(mCb),
                                                                              m_recurring(mRecurring) {
        m_next = GetCurrentMs() + mMs;
    }

    bool Timer::refresh() {

        auto mgr = m_manager;
        if (!mgr) return false;

        TimerManager::MutexType::WLock lock(mgr->m_mutex);
        auto itr = mgr->m_timers.find(shared_from_this());
        if (itr != mgr->m_timers.end()) {
            mgr->m_timers.erase(itr);
        }
        if (m_recurring) {
            m_next = GetCurrentMs() + m_ms;
            mgr->addTimerNotLock(shared_from_this());
        }
        return true;
    }

    bool Timer::cancel() {

        auto mgr = m_manager;
        if (!mgr) return false;
        TimerManager::MutexType::WLock lock(mgr->m_mutex);
        auto itr = mgr->m_timers.find(shared_from_this());
        if (itr == mgr->m_timers.end()) return false;
        mgr->m_timers.erase(itr);
        return true;

    }

    bool Timer::reset(uint64_t ms, bool from_now) {

        auto mgr = m_manager;
        if (!mgr) return false;
        TimerManager::MutexType::WLock lock(mgr->m_mutex);
        auto itr = mgr->m_timers.find(shared_from_this());
        if (itr == mgr->m_timers.end()) return false;
        mgr->m_timers.erase(itr);
        m_ms = ms;
        uint64_t start = 0;
        if (from_now) {
            start = GetCurrentMs();
        } else {
            start = m_next - ms;
        }

        m_next = start + ms;

        mgr->addTimerNotLock(shared_from_this());

        return true;

    }

    uint64_t Timer::getMNext() const {
        return m_next;
    }

    const Timer::CallBack &Timer::getMCb() const {
        return m_cb;
    }

    bool Timer::isMRecurring() const {
        return m_recurring;
    }

    TimerManager *Timer::getMManager() const {
        return m_manager;
    }

    void Timer::setMManager(TimerManager *mManager) {
        m_manager = mManager;
    }

    void TimerManager::addTimer(uint64_t ms, TimerManager::CallBack cb, bool recurring) {
        MutexType::WLock lock(m_mutex);
        addTimerNotLock(std::make_shared<Timer>(ms, cb, recurring));
    }

    static void RunConditionCallBack(TimerManager::CallBack cb, std::weak_ptr<void> con) {
        auto ptr = con.lock();
        if (ptr) {
            cb();
        }
    }

    void
    TimerManager::addConditionTimer(uint64_t ms, TimerManager::CallBack cb, std::weak_ptr<void> con, bool recurring) {
        addTimer(ms, std::bind(RunConditionCallBack, std::move(cb), std::move(con)), recurring);
    }

    void TimerManager::addTimerNotLock(Timer::ptr timer) {
        if (!timer->getMCb()) return;
        auto itr = m_timers.insert(timer).first;
        timer->setMManager(this);
        if (itr == m_timers.begin()) {
            this->onTimerInsertedAtFront();
        }

    }

    uint64_t TimerManager::getNextTimer() {
        MutexType::RLock lock(m_mutex);
        if (m_timers.empty()) return ~0ull;// 即1111111111111111，因为是无符号数，所以不存在返回-1的
        uint64_t now_s = GetCurrentMs();
        if (now_s > (*m_timers.begin())->getMNext()) {
            return 0;
        }
        return (*m_timers.begin())->getMNext() - now_s;
    }

    void TimerManager::listExpiredCb(std::vector<CallBack> &list) {
        MutexType::RLock lock(m_mutex);
        if (m_timers.empty()) return;
        lock.unlock();

        std::vector<Timer::ptr> tmp;
        MutexType::WLock lock2(m_mutex);
        Timer::ptr cur_timer = std::make_shared<Timer>(0, nullptr, false);
        auto itr = m_timers.lower_bound(cur_timer);
        tmp.insert(tmp.begin(), m_timers.begin(), itr);
        m_timers.erase(m_timers.begin(), itr);
        lock2.unlock();

        for (auto a : tmp) {
            if (a->isMRecurring()) {
                a->refresh();
                addTimerNotLock(a);
            }
            list.push_back(a->getMCb());
        }

    }

    bool TimerManager::hasTimer() {
        MutexType::RLock lock(m_mutex);
        return m_timers.size();
    }

}

