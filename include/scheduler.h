//
// Created by 抑~风 on 2023/1/31.
//

#ifndef CWJ_CO_NET_SCHEDULER_H
#define CWJ_CO_NET_SCHEDULER_H
#include <atomic>
#include <cstddef>
#include <deque>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "coroutine.h"
#include "log.h"
#include "macro.h"
#include "thread.h"

namespace CWJ_CO_NET {

/**
 * 使用继承该类时需要注意：
 * 1.
 * 在构造函数不能在派生类调用start(因为shared_from_this()在构造函数中调用会报错)
 * 2. 如果想要找Scheduler的线程内部停止该调度器，那么就需要使用m_auto_stop,
 * 而不能使用stop函数，因为stop函数中有join，这样会导致在子线程中调用其本身的
 * pthread_join函数，因而会抛异常
 * 3.
 * 启动一个调度器后，必须在该线程内或是其他非该线程的子线程中调用stop，以方便唤醒并退出沉睡的线程而进行资源回收；
 * 可以用下面提供的SchedulerManager来帮忙管理该操作
 * 4. Scheduler
 * 必须要用智能指针维护,因为其继承enable_shared_from_this，并用到了shared_from_this
 * */

class Scheduler : public std::enable_shared_from_this<Scheduler> {
 public:
  using ptr = std::shared_ptr<Scheduler>;
  using CallBack = Coroutine::CallBack;
  using MutexType = Mutex;

  Scheduler(size_t size, bool use_cur_thread = false,
            std::string name = "schedule", bool use_co_pool = true);

  virtual ~Scheduler();

  virtual void wake() = 0;

  virtual void idle() = 0;

  void stop();

  bool isStop();

  void auto_stop();

  virtual void start();

  //  size_t getTaskCount();

  template <typename T>
  void schedule(const T &t, int thread_id) {
    CoOrFunc task(t, thread_id);
    if (task.m_co || task.m_cb) {
      {
        //   MutexType::Lock lock(m_mutex);
        //   m_tasks.push_back(task);
        if (m_started) {
          pid_t id = Thread::GetPId();
          auto pp = checkAndGetThreadTasks(id);
          auto task_que_ptr = pp.second;
          CWJ_ASSERT(task_que_ptr);
          MutexType::Lock lock(task_que_ptr->m_mutex);
          task_que_ptr->m_task.push_back(task);
          task_que_ptr->m_task_size++;
        } else {
          MutexType::Lock lock(m_mutex);
          if (m_started) {
            lock.unlock();
            schedule(t, thread_id);
          } else {
            m_share_task_que.push_back(task);
          }
        }
      }
      wake();
    }
  }

  template <class InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool is_wake = false;
    if (m_started) {
      pid_t id = Thread::GetPId();
      auto pp = checkAndGetThreadTasks(id);
      ;
      auto task_que_ptr = pp.second;
      MutexType::Lock lock(task_que_ptr->m_mutex);

      for (auto itr = begin; itr != end; itr++) {
        if (itr->m_cb || itr->m_co) {
          task_que_ptr->m_task.push_back(*itr);
          task_que_ptr->m_task_size++;
          is_wake = true;
        }
      }
    } else {
      MutexType::Lock lock(m_mutex);
      if (m_started) {
        lock.unlock();
        schedule(begin, end);
      } else {
        for (auto itr = begin; itr != end; itr++) {
          if (itr->m_cb || itr->m_co) {
            m_share_task_que.push_back(*itr);
            CWJ_ASSERT(false);
            is_wake = true;
          }
        }
      }
    }
    if (is_wake) {
      wake();
    }
  }

 protected:
  virtual void beforeRunScheduler(){};
  virtual void afterRunScheduler(){};

  // 用于子调度模块修改线程私有意向
  static void SetConsumeIntentionId(int id);

 private:
  void run();

 public:
  struct CoOrFunc {
    CoOrFunc();

    CoOrFunc(const Coroutine::ptr &mCo, int mThreadId);

    CoOrFunc(const CallBack &mCb, int mThreadId);

    void reset() {
      m_co = nullptr;
      m_cb = nullptr;
      m_thread_id = -1;
    }

    Coroutine::ptr m_co = nullptr;
    CallBack m_cb;
    int m_thread_id = -1;
  };
  static Scheduler::ptr GetThis();

  static Coroutine::ptr GetScheduleCo();

  struct TaskQue {
    using ptr = std::shared_ptr<Scheduler::TaskQue>;
    std::list<CoOrFunc> m_task;
    std::atomic<size_t> m_task_size;
    MutexType m_mutex;
  };

  std::pair<pid_t, Scheduler::TaskQue::ptr> checkAndGetThreadTasks(pid_t id);

 private:
  std::vector<Thread::ptr> m_threads;
  //        std::list<CoOrFunc> m_tasks;
  std::unordered_map<pid_t, TaskQue::ptr> m_thread_tasks;
  std::list<CoOrFunc> m_share_task_que;

  std::string m_name;
  size_t m_thread_count = 0;
  std::atomic<size_t> m_outside_intention_ind{0};  //

  // 是否将当前线程也设置为调度线程
  bool m_use_cur_thread = false;
  bool m_use_co_pool = true;
  std::atomic<bool> m_started{false};
  std::atomic<bool> m_stopping{false};  // 用来标识调度器外部是否被停止了
  MutexType m_mutex;
  Semaphore m_start_sem{0};

 protected:
  std::atomic<bool> m_auto_stop{false};  // 用来内部停止的
  // 下面两个指标可以作为当前调度器的忙碌指标，可用于负载均衡
  std::atomic<size_t> m_active_thread_count{0};
  std::atomic<size_t> m_idle_thread_count{0};
  // 全局消费意向，主要是给子模块调整
  std::atomic<pid_t> m_global_intention_id{0};

  void wakeAllThread();
};

class SchedulerManager : NonCopyAble {
 public:
  SchedulerManager(Scheduler *scheduler, bool is_ctrl = true) {
    m_scheduler.reset(scheduler);
  };
  SchedulerManager(SchedulerManager &&tmp) {
    m_scheduler = tmp.m_scheduler;
    tmp.m_scheduler.reset();
  };
  SchedulerManager &operator=(SchedulerManager &&tmp) {
    m_scheduler = tmp.m_scheduler;
    tmp.m_scheduler.reset();
    return *this;
  };
  ~SchedulerManager(){};
  Scheduler::ptr &operator->() { return m_scheduler; }
  Scheduler::ptr &operator*() { return m_scheduler; }

 private:
  Scheduler::ptr m_scheduler = nullptr;
};

}  // namespace CWJ_CO_NET

#endif  // CWJ_CO_NET_SCHEDULER_H
