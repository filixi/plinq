#pragma once

#include <atomic>
#include <type_traits>

#include "../utility.h"
#include "element-aggregator-payload.h"
#include "query-tag.h"

namespace plinq::detail {
struct count_payload {
  template <size_t idx, class Payload>
  class actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
  public:
    using output_type = size_t;
    using aggregator_type = element_aggregator_payload;

    actor() {}

    template <class Act>
    void init(Act &act) {
      count_ = &act.template get_payload<idx>().count_;
      count_->store(0);
    }

    template <class Act, class T>
    void apply(Act &, const T &) {
      ++*count_;
    }

    template <class Act>
    void done(Act &) {}

    size_t get_result() {
      return count_->load();
    }

  private:
    std::atomic<size_t> *count_ = nullptr;
  };

  std::atomic<size_t> count_{ 0 };
};

} // namespace plinq::detail
