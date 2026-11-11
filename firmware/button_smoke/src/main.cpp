#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../../shared/DebouncedButton.h"

// 三只按钮使用独立GPIO输入；仅诊断，不触发产品菜单。
struct ButtonChannel {
  int pin;
  const char *name;
  DebouncedButton input;
  uint32_t count;
};
ButtonChannel buttons[] = {{25, "BACK", {}, 0}, {26, "SWITCH", {}, 0},
                           {27, "OK", {}, 0}};
Adafruit_SSD1306 display(128, 64, &Wire, -1, 100000, 100000);
uint32_t lastHeartbeat = 0;
bool redraw = true;
const char *errorMessage = nullptr;
bool runDebounceSelfTest();

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 BUTTON_SMOKE v0.2");
  if (!runDebounceSelfTest()) {
    errorMessage = "BUTTON_ERROR DEBOUNCE_SELFTEST_FAILED";
    return;
  }
  for (auto &button : buttons) {
    pinMode(button.pin, INPUT_PULLUP);
    button.input.begin(digitalRead(button.pin) == LOW, millis());
  }
  if (!Wire.begin(21, 22, 100000)) {
    errorMessage = "BUTTON_ERROR I2C_BUS_INIT_FAILED";
    return;
  }
  Wire.setTimeOut(50);
  uint8_t address = 0;
  for (const uint8_t candidate : {0x3C, 0x3D}) {
    Wire.beginTransmission(candidate);
    const uint8_t status = Wire.endTransmission();
    Serial.printf("I2C_PROBE address=0x%02X status=%u\n",
                  static_cast<unsigned>(candidate), static_cast<unsigned>(status));
    if (status == 0) {
      address = candidate;
      break;
    }
  }
  if (address == 0) {
    errorMessage = "BUTTON_ERROR OLED_NO_ACK";
    return;
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, address, true, false)) {
    errorMessage = "BUTTON_ERROR OLED_BUFFER_INIT_FAILED";
    return;
  }
  for (const auto &button : buttons) {
    Serial.printf("BUTTON_READY pin=%d name=%s active_low=1 debounce_ms=30 initial_pressed=%u\n",
                  button.pin, button.name, static_cast<unsigned>(button.input.isPressed()));
  }
}

void loop() {
  const uint32_t now = millis();
  if (errorMessage != nullptr) {
    if (now - lastHeartbeat >= 1000) {
      lastHeartbeat = now;
      Serial.println(errorMessage);
    }
    delay(2);
    return;
  }
  for (auto &button : buttons) {
    const ButtonEdge edge = button.input.update(digitalRead(button.pin) == LOW, now);
    if (edge != ButtonEdge::None) {
      if (edge == ButtonEdge::Pressed) {
        ++button.count;
      }
      Serial.printf("BUTTON_EVENT pin=%d name=%s event=%s count=%lu uptime_ms=%lu\n",
                    button.pin, button.name,
                    edge == ButtonEdge::Pressed ? "PRESSED" : "RELEASED",
                    static_cast<unsigned long>(button.count), static_cast<unsigned long>(now));
      redraw = true;
    }
  }
  if (now - lastHeartbeat >= 1000) {
    lastHeartbeat = now;
    for (const auto &button : buttons) {
      Serial.printf("BUTTON_STATE pin=%d name=%s raw=%d pressed=%u count=%lu uptime_ms=%lu\n",
                    button.pin, button.name, digitalRead(button.pin),
                    static_cast<unsigned>(button.input.isPressed()),
                    static_cast<unsigned long>(button.count), static_cast<unsigned long>(now));
    }
    redraw = true;
  }
  if (redraw) {
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(6, 5);
    display.print("3 BUTTON TEST");
    for (unsigned index = 0; index < 3; ++index) {
      const auto &button = buttons[index];
      display.setCursor(6, 17 + index * 12);
      display.printf("%d %s %c %lu", button.pin, button.name,
                     button.input.isPressed() ? 'P' : 'R',
                     static_cast<unsigned long>(button.count));
    }
    display.setCursor(6, 53);
    display.printf("seconds: %lu", static_cast<unsigned long>(now / 1000));
    display.display();
    redraw = false;
  }
  delay(2);
}
