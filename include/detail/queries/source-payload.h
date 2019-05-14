#pragma once

#include "query-tag.h"

namespace plinq::detail {
template <class SourceContainer>
class source_payload : tag_manager<size_unchanged_tag> {
  using iterator_type = decltype(std::begin(std::declval<SourceContainer>()));
public:
  template <class Source>
  source_payload(Source &&source) : container_(std::forward<Source>(source)) {}

  template <class>
  struct output_type {
    using type = std::decay_t<decltype(*std::declval<iterator_type>())>;
  };

  template <size_t idx, class Payload>
  struct actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
    using output_type = typename source_payload::output_type<void>::type;
    using aggregator_type = element_aggregator_payload;

    actor() {}

    template <class Act>
    void init(Act &act) {
      container_ = &act.template get_payload<idx>().container_;
    }

    template <class Act>
    void apply(Act &act) {
      for (const auto &v : *container_)
        act.template apply_next<idx>(v);
      act.template done_next<idx>();
    }

    template <class Act>
    void done(Act &) {}

    // When SourceContainer is of reference type, this function must not move/copy it
    SourceContainer get_result() {
      if constexpr (std::is_reference_v<SourceContainer>)
        return *container_;
      else
        return std::move(*container_);
    }

    std::remove_reference_t<SourceContainer> *container_ = nullptr;
  };

private:
  SourceContainer container_;
};

} // namespace plinq::detail
