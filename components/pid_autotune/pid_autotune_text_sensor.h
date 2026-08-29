#pragma once

#include <memory>
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/pid/pid_climate.h"
#include "esphome/components/pid/pid_autotuner.h"

namespace esphome {
namespace pid_autotune {

namespace internal {

// --- 1. Access Bypass Trick for PIDClimate (which is marked 'final') ---
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

template class MemberExtractor<AutotunerTag, &pid::PIDClimate::autotuner_>;


// --- 2. Access Bypass Trick for PIDAutotuner (not final) ---
// Since the structs OscillationFrequencyDetector and OscillationAmplitudeDetector 
// are protected types, we subclass to gain visibility, then form pointer-to-members.
class AutotunerInspector : public pid::PIDAutotuner {
public:
  static bool is_successful(pid::PIDAutotuner* tuner) {
    // Safely form pointer-to-members for the protected fields
    auto freq_ptr = &AutotunerInspector::frequency_detector_;
    auto amp_ptr  = &AutotunerInspector::amplitude_detector_;

    // Apply them to the original base pointer
    auto& freq = tuner->*freq_ptr;
    auto& amp  = tuner->*amp_ptr;

    // Check if the autotune genuinely converged
    return freq.is_increase_decrease_symmetrical() && amp.is_amplitude_convergent();
  }
};

}  // namespace internal


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
        // Evaluate the detector properties to determine if it actually succeeded
        if (internal::AutotunerInspector::is_successful(autotuner)) {
          current_status = "Finished";
        } else {
          current_status = "Failed";
        }
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