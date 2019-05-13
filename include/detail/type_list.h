#ifndef _PLINQ_DETAIL_TYPE_LIST_H_
#define _PLINQ_DETAIL_TYPE_LIST_H_

#include <type_traits>

template <class... Types>
struct type_list;

template <class TL1, class TL2>
struct concat_impl;

template <class... Types1, class... Types2>
struct concat_impl<type_list<Types1...>, type_list<Types2...>> {
  using type = type_list<Types1..., Types2...>;
};

template <class... TLs>
struct concat;

template <class... first_types, class... second_types, class... TLs>
struct concat<type_list<first_types...>, type_list<second_types...>, TLs...> {
  using type = typename concat<
    typename concat_impl<type_list<first_types...>, type_list<second_types...>>::type,
    TLs...
  >::type;
};

template <class... Types>
struct concat<type_list<Types...>> {
  using type = type_list<Types...>;
};

template <class TL, class... Types>
struct push_back : concat<TL, type_list<Types...>> {};

template <class TL, class... Types>
struct push_front : concat<type_list<Types...>, TL> {};

template <template <class> class Transform, class TL>
struct transform;

template <template <class> class Transform, class... Types>
struct transform<Transform, type_list<Types...>> {
  using type = type_list<typename Transform<Types>::type...>;
};

template <template <class, size_t> class Transform, class TL, class IndexSeq>
struct indexed_transform_impl;

template <template <class, size_t> class Transform, class... Types, size_t... idxs>
struct indexed_transform_impl<Transform, type_list<Types...>, std::index_sequence<idxs...>> {
  using type = type_list<typename Transform<Types, idxs>::type...>;
};

template <template <class, size_t> class Transform, class TL>
struct indexed_transform : indexed_transform_impl<Transform, TL, std::make_index_sequence<TL::size_v>> {};

template <class... Types>
struct type_list {
  constexpr static size_t size_v = sizeof...(Types);

  template <class... TLs>
  using concat_t = typename concat<type_list, TLs...>::type;

  template <class... Types>
  using push_back_t = typename push_back<type_list, Types...>::type;

  template <class... Types>
  using push_front_t = typename push_front<type_list, Types...>::type;

  template <template <class> class TransType>
  using transform_t = typename transform<TransType, type_list>::type;

  template <template <class, size_t> class TransType>
  using indexed_transform_t = typename indexed_transform<TransType, type_list>::type;

  using tuple_t = std::tuple<Types...>;

  template <size_t idx>
  using get_front_t = std::tuple_element_t<idx, tuple_t>;

  template <size_t idx>
  using get_back_t = get_front_t<size_v - idx - 1>;
};

#endif // _PLINQ_DETAIL_TYPE_LIST_H_
