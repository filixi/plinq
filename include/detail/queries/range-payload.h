#pragma once

#include <atomic>
#include <type_traits>
#include <vector>

#include "../utility.h"
#include "element-aggregator-payload.h"
#include "query-tag.h"

template <class IntType, class StepType>
class range_payload : tag_manager<size_unchanged_tag> {
  static_assert(std::is_integral_v<IntType>, "Internal: IntType must be of integral type.");
  static_assert(std::is_integral_v<StepType>, "Internal: StepType must be of integral type.");
public:
  template <class Source>
  range_payload(IntType first, IntType last, IntType step)
    : first_(first), last_(last), step_(step) {}

  template <class>
  struct output_type {
    using type = IntType;
  };

  template <size_t idx, class Payload>
  struct actor {
    static_assert(is_payload_v<Payload>, "Internal: Payload must be payload type.");
    using output_type = typename range_payload::output_type<void>::type;
    using aggregator_type = element_aggregator_payload;

    actor() {}

    template <class Act>
    void init(Act &act) {
      const auto &payload = act.template get_payload<idx>();
      ite_ = &payload.ite_;
      ite_->store(payload.first_);

      last_ = payload.last_;
      step_ = payload.step_;
    }

    template <class Act>
    void apply(Act &act) {
      if (step_ > 0) {
        for (; ite_->load() < last_; ite_ += step_)
          act.template apply_next<idx>(ite_->load());
      }
      else {
        for (; ite_->load() > last_; ite_ += step_)
          act.template apply_next<idx>(ite_->load());
      }

      act.template done_next<idx>();
    }

    template <class Act>
    void done(Act &) {}

    decltype(auto) get_result() {
      std::vector<IntType> result;
      if (step_ > 0) {
        for (; ite_->load() < last_; ite_ += step_)
          result.push_back(ite_.load());
      }
      else {
        for (; ite_->load() > last_; ite_ += step_)
          result.push_back(ite_.load());
      }

      return result;
    }

    std::atomic<IntType> *ite_ = nullptr;
    IntType last_;
    StepType step_;
  };

private:
  IntType first_;
  IntType last_;
  StepType step_;

  std::atomic<IntType> ite_;
};
