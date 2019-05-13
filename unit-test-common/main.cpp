

#include <iostream>
#include <type_traits>
#include <vector>

#include "plinq.h"

struct query {
  template <size_t, class>
  struct actor {

  };
};

void test_payload() {
  using a = type_list<int, double>;
  using b = type_list<int *>;

  auto d = empty_payload();
  auto x = std::move(d).push_back<query>();
  auto view = x.get_view();
}

int main()
{
  std::vector<int> a{ 1, 2, 3 };
  std::cout << linq(a).count() << std::endl;

  auto v = linq(a).select([](auto &v) { return v; }).apply();
  std::cout << &a << std::endl;
  std::cout << &linq(a).apply() << std::endl;

  // Danger! vv is a dangling pointer.
  auto vv = linq({ 1, 2, 3 }).apply();
  vv = nullptr;

  auto vvv = linq(std::vector<int>(a)).apply();
  static_assert(std::is_same_v<decltype(vvv), std::vector<int>>);

  for (auto &x : vvv)
    std::cout << x;

  test_payload();
  return 0;
}
