#pragma once

#include <atomic>
#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <type_traits>
#include <vector>

#include "../concurrentqueue.h"
#include "../thread-pool.h"
#include "../utility.h"
#include "query-tag.h"
#include "sequence-aggregator-payload.h"

class concurrent_scheduler_payload : tag_manager<size_unchanged_tag> {
public:
  template <class InputType>
  struct output_type {
    using type = std::decay_t<InputType>;
  };

  template <size_t idx, class Payload>
  class actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
    using input_type = typename Payload::template output_type<idx - 1>::type;

  public:
    using output_type = input_type;
    using aggregator_type = sequence_aggregator_payload<output_type>;

    actor() {}

    template <class Act>
    void init(Act &act) {
      sync_ = act.template get_payload<idx>()->sync_;

      auto consumer = [this]() {
        const input_type *p = nullptr;
        ++sync_.task_in_progress_;
        do {
          while (!sync_.terminate_.load() && cache_.try_dequeue(p))
            act.template apply_next<idx>(std::as_const(*p));

          std::unique_lock<std::mutex> lock(sync_.mtx_);
          sync_.task_pushed_.wait(lock, [this]() { return cache_.size_approx() != 0 || sync_.stop_.load(); });
        } while (!sync_.stop_.load());
        --sync_.task_in_progress_;
      };

      auto &pool = act.template get_payload<idx>().pool_;
      sync_.consumers_.resize(pool_->pool_size());
      for (auto &task : sync_.consumers_)
        task = std::packaged_task<void()>(consumer);

      pool_->push_task(sync_.consumers_.begin(), sync_.consumers_.end());
    }

    template <class Act, class T>
    void apply(Act &, T &&t) {
      if constexpr (std::is_rvalue_reference_v<T &&>) {
        // if argument of t is rvalue, holds the object
        std::lock_guard guard(mtx_);
        source_.emplace_back(std::forward<T>(t));
        const bool succeed = cache_.enqueue(&source_.back());

        if (!succeed)
          throw std::runtime_error("Internal: Failed in enqueuing");
      }
      else {
        const bool succeed = cache_.enqueue(&source_.back());

        if (!succeed)
          throw std::runtime_error("Internal: Failed in enqueuing");
      }

      sync_.task_pushed_.notify_one();
    }

    template <class Act>
    void done(Act &act) {
      sync_.stop_.store(true);
      sync_.task_pushed_.notify_all();

      try {
        for (auto &f : sync_.consumers_)
          f.get_future().get();
      }
      catch (...) {}

      act.done_next();
    }

    void get_result() {
      throw std::exception("Internal, scheduler has no get_result.");
    }

  private:
    moodycamel::ConcurrentQueue<const input_type *> cache_;

    std::mutex mtx_;
    std::vector<input_type> source_;

    struct sync_util *sync_ = nullptr;
  };

private:
  std::shared_ptr<thread_pool> pool_ = detail::get_global_pool();

  struct sync_util {
    std::atomic<bool> stop_{ false };
    std::atomic<bool> terminate_{ false };
    std::atomic<size_t> task_in_progress_{ 0 };

    std::condition_variable task_pushed_;
    std::mutex mtx_;

    std::vector<std::packaged_task<void()>> consumers_;

    std::promise<void> all_done_;
  } sync_;

};