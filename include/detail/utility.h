#ifndef _PLINQ_DETAIL_UTILITY_H_
#define _PLINQ_DETAIL_UTILITY_H_

#include <memory>

struct match_not_found {};

template <class First, class... Rest>
struct match_impl {
  using type = std::conditional_t<
    std::tuple_element_t<0, First>::value,
    std::tuple_element_t<1, First>,
    typename match_impl<Rest...>::type
  >;
};

template <class Last>
struct match_impl<Last> {
  using type = std::conditional_t<
    std::tuple_element_t<0, Last>::value,
    std::tuple_element_t<1, Last>,
    match_not_found
  >;
};

template <class... X>
struct match {
  using type = typename match_impl<X...>::type;

  static_assert(!std::is_same_v<type, match_not_found>, "Match failed, all pred is false.");
};

template <class, class>
class payload;

template <class T>
struct is_payload : std::false_type {};
template <class T1, class T2>
struct is_payload<payload<T1, T2>> : std::true_type {};
template <class T>
inline constexpr bool is_payload_v = is_payload<T>::value;
static_assert(is_payload_v<payload<void, void>>, "Internal: is_payload_v test failed.");
static_assert(!is_payload_v<int>, "Internal: is_payload_v test failed.");

#endif // _PLINQ_DETAIL_UTILITY_H_
