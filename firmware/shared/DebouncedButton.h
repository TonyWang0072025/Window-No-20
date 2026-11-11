#pragma once
#include <stdint.h>

enum class ButtonEdge { None, Pressed, Released };

// 只处理输入状态与时间，不依赖GPIO、屏幕或产品功能。
class DebouncedButton {
 public:
  void begin(bool pressed, uint32_t now) {
    rawPressed_ = stablePressed_ = pressed;
    changedAt_ = now;
  }

  ButtonEdge update(bool pressed, uint32_t now) {
    if (pressed != rawPressed_) {
      rawPressed_ = pressed;
      changedAt_ = now;
    }
    if (rawPressed_ != stablePressed_ &&
        static_cast<uint32_t>(now - changedAt_) >= 30) {
      stablePressed_ = rawPressed_;
      return stablePressed_ ? ButtonEdge::Pressed : ButtonEdge::Released;
    }
    return ButtonEdge::None;
  }

  bool isPressed() const { return stablePressed_; }

 private:
  bool rawPressed_ = false;
  bool stablePressed_ = false;
  uint32_t changedAt_ = 0;
};
