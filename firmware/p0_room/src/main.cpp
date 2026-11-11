#include <Arduino.h>
#include <Wire.h>
#include <esp_timer.h>
#include "../../shared/DebouncedButton.h"
#include "RoomView.h"

struct Input { int pin; Key key; DebouncedButton debounce; };
Input inputs[] = {{25, Key::Back, {}}, {26, Key::Switch, {}}, {27, Key::Confirm, {}}};
Adafruit_SSD1306 display(128, 64, &Wire, -1, 100000, 100000);
RoomModel model;
bool dirty = true;
const char *error = nullptr;
uint32_t lastHeartbeat = 0;
uint16_t lastMinute = 0xffff;
bool runRoomTests();

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 P0_ROOM v0.1");
  if (!runRoomTests()) { error = "P0_ERROR SELFTEST_FAILED"; return; }
  for (auto &input : inputs) {
    pinMode(input.pin, INPUT_PULLUP);
    input.debounce.begin(digitalRead(input.pin) == LOW, millis());
  }
  if (!Wire.begin(21, 22, 100000)) { error = "P0_ERROR I2C_INIT_FAILED"; return; }
  Wire.setTimeOut(50);
  uint8_t address = 0;
  for (uint8_t candidate : {0x3C, 0x3D}) {
    Wire.beginTransmission(candidate);
    const auto status = Wire.endTransmission();
    Serial.printf("I2C_PROBE address=0x%02X status=%u\n", static_cast<unsigned>(candidate), static_cast<unsigned>(status));
    if (status == 0) { address = candidate; break; }
  }
  if (!address) { error = "P0_ERROR OLED_NO_ACK"; return; }
  if (!display.begin(SSD1306_SWITCHCAPVCC, address, true, false)) { error = "P0_ERROR OLED_BUFFER_FAILED"; return; }
  Serial.println("P0_READY page=ROOM selected=NONE");
}

void loop() {
  const uint32_t now = millis();
  if (error) {
    if (now - lastHeartbeat >= 1000) { lastHeartbeat = now; Serial.println(error); }
    delay(2);
    return;
  }
  for (auto &input : inputs) {
    if (input.debounce.update(digitalRead(input.pin) == LOW, now) == ButtonEdge::Pressed) {
      const bool changed = model.handle(input.key);
      Serial.printf("KEY pin=%d changed=%u page=%s selected=%s uptime_ms=%lu\n", input.pin,
                    static_cast<unsigned>(changed), model.pageName(), model.selectionName(), static_cast<unsigned long>(now));
      dirty |= changed;
    }
  }
  // 仅基础本地走时；校准、昼夜天气与闲置管理在P3实现。
  const uint16_t minute = (720ULL + static_cast<uint64_t>(esp_timer_get_time()) / 60000000ULL) % 1440ULL;
  if (minute != lastMinute) { lastMinute = minute; dirty = true; }
  if (dirty) { drawRoomView(display, model, minute); dirty = false; }
  if (now - lastHeartbeat >= 1000) {
    lastHeartbeat = now;
    Serial.printf("P0_STATE page=%s selected=%s uptime_ms=%lu\n", model.pageName(), model.selectionName(), static_cast<unsigned long>(now));
  }
  delay(2);
}
