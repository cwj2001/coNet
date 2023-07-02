//
// Created by 抑~风 on 2023-03-04.
//

#include "copool.h"

#include <algorithm>
#include <cmath>

#include "config.h"
#include "log.h"
#include "macro.h"
namespace CWJ_CO_NET {

static auto g_logger = GET_LOGGER("system");
static thread_local std::queue<Coroutine::ptr> t_co_que;
static const int THREAD_MAX_CO = 512;
static const int CENTER_MAX_CO = 1024;
static auto g_thread_max_co =
    GET_CONFIG_MGR()
        -> lookup<int>("copool.thread_max_co", THREAD_MAX_CO, "thread_max_co");
static auto g_center_max_co =
    GET_CONFIG_MGR()
        -> lookup<int>("copool.center_max_co", CENTER_MAX_CO, "center_max_co");

static int init_t = []() {
  int key = 0x1234;

  g_center_max_co->addCallBack(key, [](const int& oldVal, const int& newVal) {
    if (oldVal == newVal) return;
    auto pool = CoPoolMgr::GetInstance();
    pool->setMCenterMaxCo(newVal);
  });

  g_thread_max_co->addCallBack(key, [](const int& oldVal, const int& newVal) {
    if (oldVal == newVal) return;
    auto pool = CoPoolMgr::GetInstance();
    pool->setMThreadMaxCo(newVal);
  });

  auto pool = CoPoolMgr ::GetInstance();
  pool->setMCenterMaxCo(CENTER_MAX_CO);
  pool->setMThreadMaxCo(THREAD_MAX_CO);

  return 0;
}();

Coroutine::ptr CoroutinePool::allocCo(CoroutinePool::CallBack cb,
                                      size_t stack_size, bool use_scheduler) {
  // 私有空间有
  if (t_co_que.size()) {
    auto c = t_co_que.front();
    t_co_que.pop();
    c->reset(cb, stack_size, use_scheduler);
    //            CWJ_ASSERT(false);
    return c;
  }

  // 共享空间

  if (m_co_center_size > 0) {
    MutexType::Lock lock(m_mutex);
    if (m_co_center_size > 0) {
      auto c = m_co_center.front();
      m_co_center.pop();
      m_co_center_size -= 1;
      c->reset(cb, stack_size, use_scheduler);
      return c;
    }
  }

  // 创建新协程

  return std::make_shared<Coroutine>(cb, stack_size, use_scheduler);
}

void CoroutinePool::deallocCo(Coroutine::ptr co) {
  // 处理私有空间

  if (m_thread_max_co > t_co_que.size()) {
    t_co_que.push(co);
    return;
  }

  if (m_center_max_co > m_co_center.size()) {
    MutexType::Lock lock(m_mutex);
    if (m_thread_max_co > m_co_center.size()) {
      m_co_center.push(co);
      m_co_center_size += 1;
      return;
    }
  }

  co.reset();

  return;
}

CoroutinePool::CoroutinePool() {}

const std::atomic<uint64_t>& CoroutinePool::getMThreadMaxCo() const {
  return m_thread_max_co;
}

void CoroutinePool::setMThreadMaxCo(uint64_t mThreadMaxCo) {
  if (mThreadMaxCo > 0) m_thread_max_co = mThreadMaxCo;
}

const std::atomic<uint64_t>& CoroutinePool::getMCenterMaxCo() const {
  return m_center_max_co;
}

void CoroutinePool::setMCenterMaxCo(uint64_t mCenterMaxCo) {
  if (mCenterMaxCo > 0) m_center_max_co = mCenterMaxCo;
}
}  // namespace CWJ_CO_NET