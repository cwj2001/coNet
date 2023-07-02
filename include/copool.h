//
// Created by 抑~风 on 2023-03-04.
//

#ifndef CWJ_CO_NET_COPOOL_H
#define CWJ_CO_NET_COPOOL_H

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>

#include "coroutine.h"
#include "mutex.h"
#include "singleton.h"
namespace CWJ_CO_NET {

class CoroutinePool {
 public:
  using CallBack = std::function<void()>;
  using MutexType = Mutex;

  friend class Singleton<CoroutinePool>;

  Coroutine::ptr allocCo(CallBack cb, size_t stack_size, bool use_scheduler);
  void deallocCo(Coroutine::ptr co);

  const std::atomic<uint64_t> &getMThreadMaxCo() const;

  void setMThreadMaxCo(uint64_t mThreadMaxCo);

  const std::atomic<uint64_t> &getMCenterMaxCo() const;

  void setMCenterMaxCo(uint64_t mCenterMaxCo);

 private:
  CoroutinePool();

 private:
  std::atomic<uint64_t> m_thread_max_co{16};
  std::atomic<uint64_t> m_center_max_co{32};

  // 共享区
  std::queue<Coroutine::ptr> m_co_center;
  std::atomic<uint64_t> m_co_center_size{0};
  MutexType m_mutex;
};

using CoPoolMgr = Singleton<CoroutinePool>;

}  // namespace CWJ_CO_NET

#endif  // CWJ_CO_NET_COPOOL_H
