#pragma once

#include "../utility.h"
#include "query-tag.h"

struct element_aggregator_payload {
  template <size_t, class Payload>
  struct actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");

    actor() {}

    template <class Act>
    void init(Act &) {}

    template <class Act, class... Args>
    void apply(Act &, const Args&...) {}

    template <class Act>
    void done(Act &) {}

    template <class LastActor>
    decltype(auto) get_result(LastActor &actor) {
      return actor.get_result();
    }

  };
};
