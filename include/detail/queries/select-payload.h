#pragma once

#include <exception>
#include <type_traits>

#include "../utility.h"
#include "query-tag.h"
#include "sequence-aggregator-payload.h"

namespace plinq::detail {
template <class Fn>
struct select_payload : tag_manager<size_unchanged_tag> {
  template <class Callable>
  select_payload(Callable &&callable) : fn_(callable) {}

  template <class InputType>
  struct output_type {
    using type = std::invoke_result_t<Fn, const std::decay_t<InputType> &>;
  };

  template <size_t idx, class Payload>
  class actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
  public:
    using output_type = typename select_payload::template output_type<typename actor_output_type<Payload>::template type<idx - 1>>::type;
    using aggregator_type = sequence_aggregator_payload<output_type>;

    actor() {}

    template <class Act>
    void init(Act &) {}

    template <class Act, class T>
    void apply(Act &act, const T &t) {
      auto &fn = act.template get_payload<idx>().fn_;
      act.template apply_next<idx>(std::invoke(fn, t));
    }

    template <class Act>
    void done(Act &act) {
      act.template done_next<idx>();
    }

    void get_result() {
      throw std::exception("Internal, select has no get_result.");
    }
  };

  Fn fn_;
};

} // namespace plinq::detail
