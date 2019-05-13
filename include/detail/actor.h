#ifndef _PLINQ_DETAIL_ACTOR_H_
#define _PLINQ_DETAIL_ACTOR_H_

#include <atomic>

#include "payload.h"
#include "utility.h"

template <size_t idx = 0>
struct tuple_for_each {
  

  template <class Tuple, class Fn>
  static void apply(Tuple &&t, Fn &&fn) {
    static_assert(idx < std::tuple_size_v<std::decay_t<Tuple>>, "Internal: tuple idx out of bound.");

    std::invoke(std::forward<Fn>(fn), std::get<idx>(std::forward<Tuple>(t)));
    if constexpr (idx + 1 < std::tuple_size_v<std::decay_t<Tuple>>)
      tuple_for_each<idx + 1>::apply(std::forward<Tuple>(t), std::forward<Fn>(fn));
  }
};

template <class Payload>
class actors {
public:
  using tl = typename ::actor_type_list<Payload>::type;

  template <size_t idx = 0>
  using aggregator_type = typename tl::template get_back_t<idx>::aggregator_type;

  actors() {}

  template <class Act>
  void init(Act &act) {
    tuple_for_each<>::apply(actor_, [&act](auto &&x) { x.init(act); });
  }

  template <size_t idx = 0>
  auto &get_front() {
    return std::get<idx>(actor_);
  }

  template <size_t idx = 0>
  auto &get_back() {
    return get_front<tl::size_v - idx - 1>();
  }

  bool is_finished() const {
    return finished_.load();
  }

  bool set_finished() const {
    finished_.store(true);
  }

private:
  std::atomic<bool> finished_ = false;

  typename tl::tuple_t actor_;
};

template <class Payload>
struct actor_output_type {
  using tl = typename ::actor_type_list<Payload>::type;

  template <size_t idx>
  using type = typename tl::template get_front_t<idx>::output_type;
};

template <class Payload>
class act {
  //
  static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
public:
  act(Payload &payload) : payload_(&payload) {}

  // to start the calculation, return the final result
  auto apply() {
    actor_.init(*this);

    // otherwise, the result will be retrieved directly from source, no need to apply.
    // e.g. linq(vtr).apply()
    if constexpr (Payload::tl::size_v > 1) {
      auto &source = actor_.get_front<0>();
      source.apply(*this);
    }
  }

  template <size_t curr_idx, class... Args>
  void apply_next(Args&&... args) {
    actor_.get_front<curr_idx + 1>().apply(*this, std::forward<Args>(args)...);
  }

  template <size_t curr_idx, class... Args>
  void done_next(Args&&... args) {
    actor_.get_front<curr_idx + 1>().done(*this, std::forward<Args>(args)...);
  }

  template <size_t idx>
  auto &get_payload() {
    return payload_->template get_front<idx>();
  }

  decltype(auto) get_result() {
    return actor_.template get_back<0>().get_result(
        actor_.template get_back<1>()
      );
  }

private:
  Payload *payload_;
  actors<Payload> actor_;
};

#endif // _PLINQ_DETAIL_ACTOR_H_
