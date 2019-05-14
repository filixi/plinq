#pragma once

#include <vector>

#include "../utility.h"
#include "query-tag.h"

namespace plinq::detail {
template <class T>
struct sequence_aggregator_payload {
  template <size_t, class Payload>
  struct actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");

    actor() {}

    template <class Act>
    void init(Act &) {}

    template <class Act, class T>
    void apply(Act &, T &&x) {
      results_.emplace_back(std::forward<T>(x));
    }

    template <class Act>
    void done(Act &) {}

    template <class LastActor>
    std::vector<T> get_result(LastActor &) {
      return results_;
    }

    std::vector<T> results_;
  };
};

} // namespace plinq::detail
