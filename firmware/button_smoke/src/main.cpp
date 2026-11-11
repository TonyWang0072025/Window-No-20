#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../../shared/DebouncedButton.h"

// 第一只按钮：d15/d17/g15/g17，c15接GND，h17接P25。
constexpr int kButtonPin = 25;
Adafruit_SSD1306 display(128, 64, &Wire, -1, 100000, 100000);
DebouncedButton button;
uint32_t pressCount = 0;
uint32_t lastHeartbeat = 0;
bool redraw = true;
const char *errorMessage = nullptr;
bool runDebounceSelfTest();

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 BUTTON_SMOKE v0.1");
  if (!runDebounceSelfTest()) {
    errorMessage = "BUTTON_ERROR DEBOUNCE_SELFTEST_FAILED";
    return;
  }
  pinMode(kButtonPin, INPUT_PULLUP);
  button.begin(digitalRead(kButtonPin) == LOW, millis());
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
  Serial.printf("BUTTON_READY pin=25 active_low=1 debounce_ms=30 initial_pressed=%u\n",
                static_cast<unsigned>(button.isPressed()));
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
  const int raw = digitalRead(kButtonPin);
  const ButtonEdge edge = button.update(raw == LOW, now);
  if (edge != ButtonEdge::None) {
    if (edge == ButtonEdge::Pressed) {
      ++pressCount;
    }
    Serial.printf("BUTTON_EVENT pin=25 event=%s count=%lu uptime_ms=%lu\n",
                  edge == ButtonEdge::Pressed ? "PRESSED" : "RELEASED",
                  static_cast<unsigned long>(pressCount), static_cast<unsigned long>(now));
    redraw = true;
  }
  if (now - lastHeartbeat >= 1000) {
    lastHeartbeat = now;
    Serial.printf("BUTTON_STATE pin=25 raw=%d pressed=%u count=%lu uptime_ms=%lu\n",
                  raw, static_cast<unsigned>(button.isPressed()),
                  static_cast<unsigned long>(pressCount), static_cast<unsigned long>(now));
    redraw = true;
  }
  if (redraw) {
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(6, 5);
    display.print("BUTTON TEST");
    display.setCursor(6, 19);
    display.printf("P25: %s", button.isPressed() ? "PRESSED" : "RELEASED");
    display.setCursor(6, 32);
    display.printf("PRESS COUNT: %lu", static_cast<unsigned long>(pressCount));
    display.setCursor(6, 47);
    display.printf("seconds: %lu", static_cast<unsigned long>(now / 1000));
    display.display();
    redraw = false;
  }
  delay(2);
}
