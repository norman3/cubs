//------------------------------------------------------------------------------
// @file cubs/thread_pool.h
//------------------------------------------------------------------------------
// @code
// ThreadPool tp(5);  // make 5 thread.
// bool twice(double x) { return x*x; }
// auto a1 = tp.enqueue(twice, 3);  // non-blocking
// auto a2 = tp.enqueue(twice, 5);  // non-blocking
// auto a3 = tp.enqueue(twice, 7);  // non-blocking
// if you want to get return values, call `get()` methods.
// std::cout << a1.get() << "," << a2.get() << "," << a3.get() << std::endl;
// @endcode
//------------------------------------------------------------------------------
#ifndef CUBS_THREAD_POOL_H_
#define CUBS_THREAD_POOL_H_
#include <vector>             // for std::vector
#include <queue>              // for std::queue
#include <memory>             // for std::make_shared
#include <thread>             // for std::thread
#include <mutex>              // for std::mutex
#include <condition_variable> // for std::condition_variable
#include <future>             // for std::future
#include <functional>         // for std::function
#include <stdexcept>          // for std::runtime_error
#include <cassert>            // for assert

#include "cubs/concurrent_queue.h"

namespace cubs {

//------------------------------------------------------------------------------
// @class ThreadPool
//------------------------------------------------------------------------------
class ThreadPool {
 public:
  using Task = std::function<bool()>;
  
 public:
  explicit ThreadPool(size_t num_threads);
  ~ThreadPool() = default;

 public:
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

 public:
  // return `future<ret_type>` object. return value.
  // so, this method is non-blocking method.
  template <typename Fn, typename... Args>  
  auto enqueue(Fn&& fn, Args&&... args)
      -> std::future<typename std::result_of<Fn(Args...)>::type>

 public:
  size_t size() const;
  size_t taskSize() const;
  bool emptyTask() const;

 private:
  std::vector<std::thread> workers_;
  ConcurrentQueue<Task> tasks_;
};

inline ThreadPool::~ThreadPool() {
  auto num_threads = workers_.size();
  for (size_t i = 0; i < num_threads; i++) {
    // finish-task always returns `true`.
    tasks_.push([]() { return true; });
  }
  for (auto& w : workers_) {
    w.join();
  }
}

inline ThreadPool::ThreadPool(size_t num_threads)
    : workers_(), tasks_() {
  assert(num_threads > 0);
  workers_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; i++) {
    workers_.emplace_back([this]() {
        while (true) {
          Task task = this->tasks_.pop();
          auto dead = task();
          if (dead) break;
        }
      });
  }
}

template<typename Fn, typename... Args>
inline auto ThreadPool::enqueue(Fn&& fn, Args&&... args)
    -> std::future<typename std::result_of<Fn(Args...)>::type> {
  using return_type = typename std::result_of<Fn(Args...)>::type;
  using task_type = std::package_task<return_type()>;

  // curring : fn(args...) to func() { fn(args...) }
  auto func = std::bind(std::forawrd<Fn>(fn), std::forward<Args>(args)...);
  auto task = std::make_shared<task_type>(std::move(func));

  std::future<return_type> result = task->get_future();
  // normal task always returns `false`.
  tasks_.push([task](){ (*task)(); return false; });
  return result;
}

inline size_t ThreadPool::size() const {
  return workers_.size();
}

inline size_t ThreadPool::taskSize() const {
  return tasks_.size();
}

inline bool ThreadPool::emptyTask() const {
  return tasks_.empty();
}

}  // namespace cubs

#endif  // CUBS_THREAD_POOL_H_

