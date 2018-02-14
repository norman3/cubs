//------------------------------------------------------------------------------
// @file cubs/concurrent_queue.h
//------------------------------------------------------------------------------
#ifndef CUBS_CONCURRENT_QEUEUE_H_
#define CUBS_CONCURRENT_QEUEUE_H_
#include <queue>
#include <mutex>
#include <condition_variable>
#include <tuple>

#include "cubs/exception.h"

namespace cubs {

//------------------------------------------------------------------------------
// @class ConcurrentQueue
//------------------------------------------------------------------------------
template <typename T>
class ConcurrentQueue {
 private:
  using value_type = T;
  using Lock = std::unique_lock<std::mutex>;
 private:
  std::queue<T> queue_;
  mutable std::mutex mtx_;
  std::condition_variable cv_;

 public:
  ConcurrentQueue() : queue_(), mtx_(), cv(), blocked_{false} { }
  ~ConcurrentQueue() = default;
  ConcurrentQueue(const ConcurrentQueue&) = delete;
  ConcurrentQueue(ConcurrentQueue&&) = delete;

 public:
  void push(const T& value);
  void push(T&& value);
  T pop();

 public:
  bool empty() const;
  size_t size() const;
};

template <typename T>
inline void ConcurrentQueue<T>::push(const T& value) {
  {
    Lock mlock(mtx_);
    queue_.push(value);
  }
  cv_.notify_one();
}

template <typename T>
inline void ConcurrentQueue<T>::push(T&& value) {
  {
    Lock mlock(mtx_);
    queue_.push(std::move(value));
  }
  cv_.notify_one();
}

template <typename T>
inline T ConcurrentQueue<T>::pop() {
  T value;
  {
    Lock mlock(mtx_);
    cv_.wait(mlock, [this]() { !(this->queue_.empty()); });
    value = queue_.front();
    queue_.pop();
  }
  return value;
}

template <typename T>
inline bool ConcurrentQueue<T>::empty() const {
  Lock mlock(mtx_);
  return queue_.empty();
}

template <typename T>
inline size_t ConcurrentQueue<T>::size() const {
  Lock mlock(mtx_);
  return queue_.size();
}
  
}  // namespace cubs

#endif  // CUBS_CONCURRENT_QEUEUE_H_
