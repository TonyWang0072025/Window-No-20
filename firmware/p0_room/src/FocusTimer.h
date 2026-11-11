#pragma once
#include <stdint.h>
#include "../../shared/InputKey.h"

enum class FocusState { Idle, Selecting, Running, Paused, ExitConfirm, Completed, Abandoned };

// 只接受同一单调时钟的64位毫秒；没有GPIO、墙钟、加速倍率或存档依赖。
class FocusTimer {
 public:
  void enterSelection() {
    state_ = FocusState::Selecting;
    preset_ = 0;
    remainingMs_ = 0;
  }

  bool tick(uint64_t nowMs) {
    if (state_ != FocusState::Running) return false;
    const auto secondsBefore = secondsRemaining();
    const uint64_t elapsed = nowMs - lastTickMs_;
    lastTickMs_ = nowMs;
    if (elapsed >= remainingMs_) {
      remainingMs_ = 0;
      state_ = FocusState::Completed; // 先离开进行态，后续刷新不再发奖。
      constexpr uint8_t rewards[] = {1, 1, 1, 2, 3};
      stars_ += rewards[preset_]; // P1仅RAM测试数据；P2负责正式持久化闭环。
      return true;
    }
    remainingMs_ -= elapsed;
    return secondsBefore != secondsRemaining();
  }

  bool handle(Key key, uint64_t nowMs) {
    const auto before = state_;
    const bool changed = tick(nowMs);
    // 到期同刻的按键先结算，不吞奖，也不把刚出现的完成页直接确认掉。
    if (before == FocusState::Running && state_ == FocusState::Completed) return true;
    switch (state_) {
      case FocusState::Selecting:
        if (key == Key::Back) state_ = FocusState::Idle;
        else if (key == Key::Switch) preset_ = (preset_ + 1) % 5;
        else {
          remainingMs_ = static_cast<uint64_t>(selectedMinutes()) * 60000;
          lastTickMs_ = nowMs;
          state_ = FocusState::Running;
        }
        return true;
      case FocusState::Running:
      case FocusState::Paused:
        if (key == Key::Switch) return changed;
        if (key == Key::Back) {
          resumeState_ = state_;
          state_ = FocusState::ExitConfirm;
        } else if (state_ == FocusState::Running) state_ = FocusState::Paused;
        else {
          lastTickMs_ = nowMs;
          state_ = FocusState::Running;
        }
        return true;
      case FocusState::ExitConfirm:
        if (key == Key::Switch) return false;
        if (key == Key::Confirm) state_ = FocusState::Abandoned;
        else {
          state_ = resumeState_;
          lastTickMs_ = nowMs;
        }
        return true;
      case FocusState::Completed:
        if (key != Key::Confirm) return false;
        state_ = FocusState::Idle;
        return true;
      default: return false;
    }
  }

  FocusState state() const { return state_; }
  uint8_t selectedMinutes() const {
    constexpr uint8_t minutes[] = {15, 20, 30, 45, 60};
    return minutes[preset_];
  }
  uint32_t secondsRemaining() const { return static_cast<uint32_t>((remainingMs_ + 999) / 1000); }
  uint32_t stars() const { return stars_; }
  const char *stateName() const {
    constexpr const char *names[] = {"IDLE", "SELECTING", "RUNNING", "PAUSED", "EXIT_CONFIRM", "COMPLETED", "ABANDONED"};
    return names[static_cast<unsigned>(state_)];
  }

 private:
  FocusState state_ = FocusState::Idle;
  FocusState resumeState_ = FocusState::Running;
  uint8_t preset_ = 0;
  uint64_t remainingMs_ = 0;
  uint64_t lastTickMs_ = 0;
  uint32_t stars_ = 0;
};
