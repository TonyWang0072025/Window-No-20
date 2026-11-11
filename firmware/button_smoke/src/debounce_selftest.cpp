#include <Arduino.h>
#include "../../shared/DebouncedButton.h"

// 在真实ESP32上运行纯输入逻辑检查，不模拟或替代实际GPIO/按键验收。
bool runDebounceSelfTest() {
  struct Sample {
    bool pressed;
    uint32_t time;
    ButtonEdge expected;
  };
  constexpr Sample samples[] = {
      {false, 0, ButtonEdge::None},
      {true, 1, ButtonEdge::None},
      {false, 5, ButtonEdge::None},
      {true, 10, ButtonEdge::None},
      {true, 39, ButtonEdge::None},
      {true, 40, ButtonEdge::Pressed},
      {true, 1000, ButtonEdge::None},
      {false, 1001, ButtonEdge::None},
      {true, 1010, ButtonEdge::None},
      {false, 1015, ButtonEdge::None},
      {false, 1044, ButtonEdge::None},
      {false, 1045, ButtonEdge::Released},
      {true, 1200, ButtonEdge::None},
      {true, 1230, ButtonEdge::Pressed},
  };
  unsigned passed = 0;
  DebouncedButton button;
  button.begin(false, 0);
  for (const auto &sample : samples) {
    const ButtonEdge actual = button.update(sample.pressed, sample.time);
    if (actual != sample.expected) {
      Serial.printf("DEBOUNCE_SELFTEST FAIL step=%u actual=%u expected=%u\n",
                    passed, static_cast<unsigned>(actual),
                    static_cast<unsigned>(sample.expected));
      return false;
    }
    ++passed;
  }
  // 验证millis()无符号回绕，以及开机已按住不伪造一次按下。
  button.begin(false, UINT32_MAX - 20);
  if (button.update(true, UINT32_MAX - 10) != ButtonEdge::None ||
      button.update(true, 18) != ButtonEdge::None ||
      button.update(true, 19) != ButtonEdge::Pressed) {
    Serial.println("DEBOUNCE_SELFTEST FAIL wraparound");
    return false;
  }
  passed += 3;
  button.begin(true, 0);
  if (button.update(true, 1000) != ButtonEdge::None || !button.isPressed()) {
    Serial.println("DEBOUNCE_SELFTEST FAIL held_on_boot");
    return false;
  }
  ++passed;
  Serial.printf("DEBOUNCE_SELFTEST PASS checks=%u\n", passed);
  return true;
}
