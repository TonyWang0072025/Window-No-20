#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 首次点亮诊断，独立于产品界面；仅按实际套件接线使用。
constexpr int kSdaPin = 21;
constexpr int kSclPin = 22;
constexpr int kWidth = 128;
constexpr int kHeight = 64;
Adafruit_SSD1306 display(kWidth, kHeight, &Wire, -1, 100000, 100000);
uint8_t oledAddress = 0;
const char *errorMessage = nullptr;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("WINDOW_NO_20 OLED_SMOKE v0.1");

  if (!Wire.begin(kSdaPin, kSclPin, 100000)) {
    errorMessage = "OLED_ERROR I2C_BUS_INIT_FAILED";
    return;
  }
  Wire.setTimeOut(50);

  // 先测量设备应答；库的 begin() 成功不能代替 I2C 连接验证。
  for (uint8_t address : {0x3C, 0x3D}) {
    Wire.beginTransmission(address);
    const uint8_t result = Wire.endTransmission();
    Serial.printf("I2C_PROBE address=0x%02X status=%u\n",
                  static_cast<unsigned int>(address), static_cast<unsigned int>(result));
    if (result == 0) {
      oledAddress = address;
      break;
    }
  }
  if (oledAddress == 0) {
    errorMessage = "OLED_ERROR NO_ACK_ON_0x3C_OR_0x3D";
    return;
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, oledAddress, true, false)) {
    errorMessage = "OLED_ERROR BUFFER_INIT_FAILED";
    return;
  }

  display.clearDisplay();
  display.drawRect(0, 0, kWidth, kHeight, SSD1306_WHITE);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(6, 6);
  display.print("WINDOW No.20");
  display.setCursor(6, 20);
  display.print("OLED 128x64");
  display.setCursor(6, 32);
  display.printf("I2C 0x%02X", static_cast<unsigned int>(oledAddress));
  Serial.printf("OLED_INIT address=0x%02X canvas=128x64\n",
                static_cast<unsigned int>(oledAddress));
}

void loop() {
  if (errorMessage != nullptr) {
    Serial.println(errorMessage);
  } else {
    display.fillRect(6, 46, 116, 10, SSD1306_BLACK);
    display.setCursor(6, 46);
    display.printf("seconds: %lu", static_cast<unsigned long>(millis() / 1000));
    display.display();
    // 此日志表示程序调用了画面更新，是否真正可见须由实物观察确认。
    Serial.printf("OLED_UPDATE uptime_ms=%lu\n", static_cast<unsigned long>(millis()));
  }
  delay(1000);
}
