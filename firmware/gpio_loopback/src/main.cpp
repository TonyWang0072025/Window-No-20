#include <Arduino.h>
#include <driver/gpio.h>

// 独立硬件诊断：OLED 必须完全断开；白/黑线的屏幕端通过 1kΩ 相连。
// 两脚仅以开漏方式拉低或释放，内部弱上拉提供高电平，避免两路强输出对顶。
constexpr gpio_num_t kPins[] = {GPIO_NUM_21, GPIO_NUM_22};
bool pinsReady = false;
uint32_t lastHeartbeat = 0;
uint32_t sequence = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 GPIO_LOOPBACK v0.1");
  for (const auto pin : kPins) {
    gpio_set_level(pin, 1);
  }
  gpio_config_t config{};
  config.pin_bit_mask = (1ULL << GPIO_NUM_21) | (1ULL << GPIO_NUM_22);
  config.mode = GPIO_MODE_INPUT_OUTPUT_OD;
  config.pull_up_en = GPIO_PULLUP_ENABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.intr_type = GPIO_INTR_DISABLE;
  const esp_err_t result = gpio_config(&config);
  pinsReady = result == ESP_OK;
  Serial.printf("GPIO_CONFIG status=%d ready=%u; send T to test\n",
                static_cast<int>(result), static_cast<unsigned>(pinsReady));
}

void loop() {
  if (Serial.available() && Serial.read() == 'T' && pinsReady) {
    unsigned passed = 0;
    // 每个引脚分别拉低、释放，检查两个读值是否都跟随，共四项。
    for (const auto source : kPins) {
      for (const int level : {0, 1}) {
        gpio_set_level(source, level);
        delay(10);
        const int read21 = gpio_get_level(GPIO_NUM_21);
        const int read22 = gpio_get_level(GPIO_NUM_22);
        const bool ok = read21 == level && read22 == level;
        passed += ok;
        Serial.printf("CHECK source=%d level=%d read21=%d read22=%d ok=%u\n",
                      static_cast<int>(source), level, read21, read22,
                      static_cast<unsigned>(ok));
      }
    }
    Serial.printf("LOOPBACK_RESULT passed=%u total=4 result=%s\n",
                  passed, passed == 4 ? "PASS" : "FAIL");
  }
  if (millis() - lastHeartbeat >= 1000) {
    lastHeartbeat = millis();
    Serial.printf("ALIVE seq=%lu uptime_ms=%lu ready=%u\n",
                  static_cast<unsigned long>(++sequence),
                  static_cast<unsigned long>(lastHeartbeat),
                  static_cast<unsigned>(pinsReady));
  }
  delay(10);
}
