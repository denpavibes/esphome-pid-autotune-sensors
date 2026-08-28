#pragma once

#include <memory>
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/pid/pid_climate.h"

namespace esphome {
namespace pid_autotune {

namespace internal {

// --- C++ Access Bypass Trick ---
// Bypasses the 'protected' and 'final' modifiers using explicit template instantiation.
// Kept in the internal namespace to avoid polluting the component namespace.

struct AutotunerTag {
  using type = std::unique_ptr<pid::PIDAutotuner> pid::PIDClimate::*;
};

template <typename Tag>
struct MemberStorage {
  static typename Tag::type ptr;
};

template <typename Tag>
typename Tag::type MemberStorage<Tag>::ptr;

template <typename Tag, typename Tag::type P>
struct MemberExtractor {
  struct Filler {
    Filler() { MemberStorage<Tag>::ptr = P; }
  };
  static Filler filler;
};

template <typename Tag, typename Tag::type P>
typename MemberExtractor<Tag, P>::Filler MemberExtractor<Tag, P>::filler;

// Force explicit instantiation to extract the protected member
template class MemberExtractor<AutotunerTag, &pid::PIDClimate::autotuner_>;

}  // namespace internal

// Renamed from PidAutotuneTextSensor to PIDAutotuneTextSensor to match ESPHome conventions
class PIDAutotuneTextSensor : public text_sensor::TextSensor, public PollingComponent {
protected:
  pid::PIDClimate *climate_{nullptr};
  std::string last_status_{""};

public:
  void set_climate(pid::PIDClimate *climate) { climate_ = climate; }

  void update() override {
    if (this->climate_ == nullptr) {
      this->mark_failed();
      return;
    }

    std::string current_status = "Off";
    
    // Grab the pointer-to-member from our static internal storage
    auto ptr_to_member = internal::MemberStorage<internal::AutotunerTag>::ptr;
    
    // Apply it to the climate instance to get the protected unique_ptr
    auto &autotuner_ptr = this->climate_->*ptr_to_member;
    
    // Extract the raw pointer for logic checks
    pid::PIDAutotuner *autotuner = autotuner_ptr.get();

    if (autotuner != nullptr) {
      if (autotuner->is_finished()) {
        current_status = "Finished";
      } else {
        current_status = "Running";
      }
    }

    // Only publish if the state has changed
    if (current_status != this->last_status_) {
      this->publish_state(current_status);
      this->last_status_ = current_status;
    }
  }
};

}  // namespace pid_autotune
}  // namespace esphome