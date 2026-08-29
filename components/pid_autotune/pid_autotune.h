#pragma once

#include <memory>
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
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
class AutotunerInspector : public pid::PIDAutotuner {
public:
  static bool is_successful(pid::PIDAutotuner* tuner) {
    auto freq_ptr = &AutotunerInspector::frequency_detector_;
    auto amp_ptr  = &AutotunerInspector::amplitude_detector_;
    auto& freq = tuner->*freq_ptr;
    auto& amp  = tuner->*amp_ptr;
    return freq.is_increase_decrease_symmetrical() && amp.is_amplitude_convergent();
  }

  static uint32_t get_phase_count(pid::PIDAutotuner* tuner) {
    auto relay_ptr = &AutotunerInspector::relay_function_;
    auto& relay = tuner->*relay_ptr;
    return relay.phase_count;
  }
};

}  // namespace internal


// --- Text Sensor: Status ---
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
    
    auto ptr_to_member = internal::MemberStorage<internal::AutotunerTag>::ptr;
    auto &autotuner_ptr = this->climate_->*ptr_to_member;
    pid::PIDAutotuner *autotuner = autotuner_ptr.get();

    if (autotuner != nullptr) {
      if (autotuner->is_finished()) {
        if (internal::AutotunerInspector::is_successful(autotuner)) {
          current_status = "Finished";
        } else {
          current_status = "Failed";
        }
      } else {
        current_status = "Running";
      }
    }

    if (current_status != this->last_status_) {
      this->publish_state(current_status);
      this->last_status_ = current_status;
    }
  }
};

// --- Numeric Sensor: Phase Counter ---
class PIDAutotuneSensor : public sensor::Sensor, public PollingComponent {
protected:
  pid::PIDClimate *climate_{nullptr};

public:
  void set_climate(pid::PIDClimate *climate) { climate_ = climate; }

  void update() override {
    if (this->climate_ == nullptr) {
      this->mark_failed();
      return;
    }

    auto ptr_to_member = internal::MemberStorage<internal::AutotunerTag>::ptr;
    auto &autotuner_ptr = this->climate_->*ptr_to_member;
    pid::PIDAutotuner *autotuner = autotuner_ptr.get();

    float current_phase = 0.0f; // Default to 0 when off/finished

    if (autotuner != nullptr && !autotuner->is_finished()) {
      current_phase = (float)internal::AutotunerInspector::get_phase_count(autotuner);
    }

    // Only publish if the state has changed
    if (!this->has_state() || this->get_state() != current_phase) {
      this->publish_state(current_phase);
    }
  }
};

}  // namespace pid_autotune
}  // namespace esphome
