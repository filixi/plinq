#ifndef _PLINQ_DETAIL_PAYLOAD_H_
#define _PLINQ_DETAIL_PAYLOAD_H_

#include <memory>
#include <tuple>
#include <type_traits>

#include "type_list.h"

struct unique_ptr_decorate_tag {
  template <class T>
  struct transform {
    using type = std::unique_ptr<T>;
  };
};

struct raw_ptr_decorate_tag {
  template <class T>
  struct transform {
    using type = std::add_pointer_t<T>;
  };
};

template <class Decorate, class TL>
class payload {
public:
  template <class Decorate, class TL>
  friend class payload;

  using tl = TL;
  using payload_storage = typename TL::template transform_t<Decorate::transform>::tuple_t;

  template <class T, size_t idx>
  struct actor_of {
    using type = typename T::template actor<idx, payload>;
  };

  template <class Query, class... Args>
  auto push_back(Args&&... args) && {
    static_assert(std::is_same_v<Decorate, unique_ptr_decorate_tag>, "Internal: must be unique_ptr payload.");
    payload<Decorate, TL::template push_back_t<Query>> new_payload;
    new_payload.payloads_ = std::tuple_cat(
      std::move(payloads_),
      std::make_tuple(std::make_unique<Query>(std::forward<Args>(args)...))
    );

    return new_payload;
  }

  template <size_t idx = 0>
  auto &get_front() {
    return *std::get<idx>(payloads_);
  }

  template <size_t idx = 0>
  auto &get_back() {
    return get_front<TL::size_v - idx - 1>();
  }

  auto get_view() {
    if constexpr (std::is_same_v<Decorate, unique_ptr_decorate_tag>) {
      payload<raw_ptr_decorate_tag, TL> view;
      view.payloads_ = get_view_impl(std::make_index_sequence<TL::size_v>());
      return view;
    } else if constexpr (std::is_same_v<Decorate, raw_ptr_decorate_tag>) {
      return *this;
    } else {
      static_assert(false, "Unknown decorate tag");
    }
  }

private:
  template <size_t... idxs>
  auto get_view_impl(std::index_sequence<idxs...>) {
    return std::make_tuple(std::get<idxs>(payloads_).get() ...);
  }

  payload_storage payloads_;
};

auto empty_payload() {
  return payload<unique_ptr_decorate_tag, type_list<>>();
}

template <class Payload>
struct actor_type_list {
  using type = typename Payload::tl::template indexed_transform_t<Payload::actor_of>;
};

#endif // _PLINQ_DETAIL_PAYLOAD_H_
