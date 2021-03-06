#pragma once

#include <tuple>
#include <type_traits>

namespace plinq::detail {
template <class... tags>
struct tag_manager {
  using tags_list = std::tuple<tags...>;
};

template <class T, class Tag>
struct has_tag_impl;

template <class... Tags, class Tag>
struct has_tag_impl<std::tuple<Tags...>, Tag>
  : std::conditional_t<(std::is_same_v<Tags, Tag> || ...), std::true_type, std::false_type> {};

template <class T, class Tag>
struct has_tag : has_tag_impl<typename T::tags_list, Tag> {};

template <class T, class Tag>
inline constexpr bool has_tag_v = has_tag<T, Tag>::value;

} // namespace plinq::detail
