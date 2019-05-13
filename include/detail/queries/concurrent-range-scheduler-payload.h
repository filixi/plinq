#pragma once

#include <atomic>
#include <exception>
#include <future>
#include <type_traits>
#include <utility>
#include <vector>

#include "../thread-pool.h"
#include "../utility.h"
#include "query-tag.h"

class concurrent_range_scheduler_payload : tag_manager<size_unchanged_tag> {
public:
  template <class>
  struct output_type {
    using type = void;
  };

  template <size_t idx, class Payload>
  class actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");

  public:
    using output_type = void;
    using aggregator_type = void;

    actor() {}

    template <class Act>
    void init(Act &) {}

    template <class Act, class T>
    void apply(Act &act) {
      auto &pool = act.template get_payload<idx>().pool_;

      std::vector<std::future<void>> syncs;
      for (int i = 0; i < pool->size(); ++i) {
        std::packaged_task<void(void)> task([&act]() {
          act.template apply_next<idx>();
        });
        syncs.push_back(task.get_future());

        pool->push_task(std::move(task));
      }

      for (auto &f : syncs)
        f.get();

      act.done_next();
    }

    template <class Act>
    void done(Act &act) {
      act.done_next();
    }

    void get_result() {
      throw std::exception("Internal, scheduler has no get_result.");
    }

  };

private:
  std::shared_ptr<thread_pool> pool = detail::get_global_pool();
};