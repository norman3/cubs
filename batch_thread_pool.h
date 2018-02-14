//------------------------------------------------------------------------------
// @file cubs/batch_thread_pool.h
//------------------------------------------------------------------------------
// @code
// BatchThreadPool<float> btp(3);
//
// for (int i = 0; i<10; i++) {
// 
// }
// @endcode
//------------------------------------------------------------------------------
#ifndef CUBS_BATCH_THREAD_POOL_H_
#define CUBS_BATCH_THREAD_POOL_H_
#include <type_traits>        // for several traits
#include <future>             // for std::future
#include <mutex>              // for std::mutex

namespace cubs {

//------------------------------------------------------------------------------
// @class BatchThreadPool<T>
//------------------------------------------------------------------------------
template <typename T>
class BatchThreadPool {
 public:
  using Lock = std::unique_lock<std::mutex>;
  using value_type = T;
  static_assert(!std::is_reference<T>::value,
                "do not use reference type with BatchThreadPool");

 private:
  ThreadPool& pool_;
  std::vector<std::future<value_type>> buf_;
  mutable std::mutex mtx_;  

 public:
  explicit BatchThreadPool(ThreadPool& pool, size_t reservedBufferSize = 0)
      : pool_{pool}, buf_(), mtx_() {
    if (reservedBufferSize > 0)
      buf_.reserve(reservedBufferSize);
  }
  BatchThreadPool(const BatchThreadPool& pool) = delete;
  BatchThreadPool(BatchThreadPool&& pool) = delete;
  BatchThreadPool& operator=(const BatchThreadPool& pool) = delete;
  BatchThreadPool& operator=(BatchThreadPool&& pool) = delete;

 public:
  template <typename Fn, typename... Args>  
  void enqueue(Fn&& fn, Args&&... args) {
    using return_type = typename std::result_of<Fn(Args...)>::type;
    auto fut = pool_.enqueue(std::forward<Fn>(fn), std::forward<Args>(args)...);
    {
      Lock mlock(mtx_);
      buf_.push_back(std::move(fut));
    }
  }

  std::vector<T> getAll() {
    std::vector<T> result;
    result.reserve(buf_.size());
    // slow if buf_ vector is very large.
    {
      Lock mlock(mtx_);
      for (auto& val : buf_)
        result.push_back(val.get());
      buf_.clear();
    }
    return result;
  }
};

//------------------------------------------------------------------------------
// @class BatchThreadPool<void>
//------------------------------------------------------------------------------
template <>
class BatchThreadPool<void> {
 public:
  using Lock = std::unique_lock<std::mutex>;
  using value_type = void;

 private:
  ThreadPool& pool_;
  std::vector<std::future<value_type>> buf_;
  mutable std::mutex mtx_;  

 public:
  explicit BatchThreadPool(ThreadPool& pool, size_t reservedBufferSize = 0)
      : pool_{pool}, buf_(), mtx_() {
    if (reservedSize > 0)
      buf_.reserve(reserveBufferSize);
  }
  BatchThreadPool(const BatchThreadPool& pool) = delete;
  BatchThreadPool(BatchThreadPool&& pool) = delete;
  BatchThreadPool& operator=(const BatchThreadPool& pool) = delete;
  BatchThreadPool& operator=(BatchThreadPool&& pool) = delete;

 public:
  template <typename Fn, typename... Args>  
  void enqueue(Fn&& fn, Args&&... args) {
    using return_type = typename std::result_of<Fn(Args...)>::type;
    static_assert(std::is_void<return_type>::value,
                  "invalid fn() return type. (use 'void' type)");
    auto fut = pool_.enqueue(std::forward<Fn>(fn), std::forward<Args>(args)...);
    {
      Lock mlock(mtx_);
      buf_.push_back(std::move(fut));
    }
  }

  void getAll() {
    // slow if buf_ vector is very large.
    Lock mlock(mtx_);
    for (auto& val : buf_)
      val.get();
  }
};



}  // namespace cubs

#endif  // CUBS_BATCH_THREAD_POOL_H_
