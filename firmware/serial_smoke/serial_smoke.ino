#include <Arduino.h>

// 第一次烧录只验证 USB 串口通信，不连接或驱动外设。
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 SERIAL_SMOKE v0.1");
  Serial.printf("chip=%s revision=%u flash_bytes=%lu\n",
                ESP.getChipModel(), static_cast<unsigned int>(ESP.getChipRevision()),
                static_cast<unsigned long>(ESP.getFlashChipSize()));
}

void loop() {
  Serial.printf("RUNNING uptime_ms=%lu\n", static_cast<unsigned long>(millis()));
  delay(1000);
}
