#pragma once

#include <mutex>
#include <thread>
#include <future>
#include <vector>
#include <list>
#include <atomic>
#include <condition_variable>

namespace plinq::detail {
class thread_pool {
public:
  thread_pool() {
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i)
      threads_.emplace_back(&thread_pool::worker, this);
  }

  ~thread_pool() {
    {
      std::lock_guard<std::mutex> guard(mtx_);
      // Why need to clear all?
      schedualed_tasks_.clear();
    }

    terminate_.store(true);
    task_pushed_.notify_all();
    for (auto &thread : threads_)
      thread.join();
  }

  void push_task(std::packaged_task<void()> &&task) {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      schedualed_tasks_.emplace_back(std::move(task));
    }
    task_pushed_.notify_one();
  }

  template <class It>
  void push_task(It first, It last) {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      std::move(first, last, std::back_inserter(schedualed_tasks_));
    }
    task_pushed_.notify_all();
  }

  void worker() {
    while (!terminate_.load()) {
      std::unique_lock<std::mutex> lock(mtx_);
      task_pushed_.wait(lock, [this]() { return !schedualed_tasks_.empty() || terminate_.load(); });

      if (terminate_.load())
        break;

      auto task = std::move(schedualed_tasks_.front());
      schedualed_tasks_.pop_front();
      lock.unlock();

      task();
    }
  }

  size_t pool_size() const {
    return threads_.size();
  }

private:
  std::list<std::packaged_task<void()>> schedualed_tasks_;

  std::vector<std::thread> threads_;

  std::mutex mtx_;
  std::condition_variable task_pushed_;
  std::atomic_bool terminate_{ false };
};

static std::shared_ptr<thread_pool> get_global_pool(bool enable_default_pool = true) {
  static std::once_flag flag;
  static std::shared_ptr<thread_pool> pool;

  if (enable_default_pool) {
    std::call_once(flag, []() { pool = std::shared_ptr<thread_pool>(); });
  } else {
    std::call_once(flag, []() {});
    pool.reset();
  }
    
  return pool;
}

} // namespace plinq::detail
