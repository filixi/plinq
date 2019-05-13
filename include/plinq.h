#ifndef _PLINQ_PLINQ_H_
#define _PLINQ_PLINQ_H_

#include <type_traits>
#include <utility>

#include "detail/actor.h"
#include "detail/queries.h"
#include "detail/utility.h"
#include "detail/thread-pool.h"

namespace detail {
template <class Payload>
class linq_impl {
public:
  linq_impl(Payload &&payload) : payload_(std::move(payload)) {}

  ~linq_impl() = default;

  // TODO: append an aggregator_payload ?
  decltype(auto) apply() && {
    using aggregator_type = typename actors<Payload>::template aggregator_type<>;

    // payload_ is moved to local, returning any reference to internal state of pyaload is fun! :)
    auto aggregator_payload = std::move(payload_).template push_back<aggregator_type>();
    auto view = aggregator_payload.get_view();

    act<decltype(view)> a(view);
    a.apply();

    return a.get_result();
  }

  auto operator()() && {
    return apply();
  }

  auto count() && {
    using payload_type = count_payload;
    return append_self<payload_type>().apply();
  }

  template <class Fn>
  auto select(Fn &&fn) && {
    using payload_type = select_payload<std::decay_t<Fn>>;
    return append_self<payload_type>(std::forward<Fn>(fn));
  }

private:
  template <class Payload>
  static auto create_linq_impl(Payload &&payload) {
    static_assert(std::is_rvalue_reference_v<Payload &&>, "Internal: payload must be moved");
    return linq_impl<Payload>(std::move(payload));
  }

  template <class Payload, class... Args>
  auto append_self(Args&&... args) {
    return create_linq_impl(std::move(payload_).template push_back<Payload>(std::forward<Args>(args)...));
  }

  linq_impl() = default;
  linq_impl(const linq_impl &) = delete;

  linq_impl &operator=(const linq_impl &) = delete;

  Payload payload_;
};

class linq_t {
public:
  template <class Source>
  auto operator()(Source &source) {
    using source_payload_type = source_payload<std::decay_t<Source> &>;
    return linq_impl(empty_payload().push_back<source_payload_type>(source));
  }

  template <class Source>
  auto operator()(Source &&source) {
    using source_payload_type = source_payload<std::decay_t<Source>>;
    return linq_impl(empty_payload().push_back<source_payload_type>(std::forward<Source>(source)));
  }

  template <class Source>
  auto operator()(const Source &source) {
    using source_payload_type = source_payload<const Source &>;
    return linq_impl(empty_payload().push_back<source_payload_type>(source));
  }

  // TODO: analyze, linq({1,2,3}).apply() will yield a dangling pointer
  template <class T, size_t bound>
  auto operator()(const T(&source)[bound]) {
    using source_payload_type = source_payload<const T(&)[bound]>;
    return linq_impl(empty_payload().push_back<source_payload_type>(source));
  }

  template <class IntType>
  auto range(IntType first, IntType last) {
    return range(first, last, first < last ? 1 : -1);
  }

  template <class IntType, class StepType>
  auto range(IntType first, IntType bound, StepType step) {
    static_assert(std::is_integral_v<IntType>, "Arguments must be of integral type.");
    static_assert(std::is_integral_v<StepType>, "Arguments must be of integral type.");
  }

  static void disable_default_thread_pool() {
    get_global_pool(false);
  }
};

struct linq_t_tag : linq_t {};
static_assert(sizeof(linq_t_tag) == 1, "Internal: linq_t must be empty.");

static_assert(std::is_trivially_constructible_v<linq_t>, "Internal: linq_t must be trivially constructible.");

} // namespace detail

inline detail::linq_t linq;

#endif // _PLINQ_PLINQ_H_
