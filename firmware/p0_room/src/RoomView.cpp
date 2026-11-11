#include "RoomView.h"

void drawRoomView(Adafruit_SSD1306 &d, const RoomModel &m, uint16_t minuteOfDay) {
  d.clearDisplay();
  d.setTextColor(SSD1306_WHITE);
  d.setTextSize(1);
  if (m.page == Page::Room) {
    d.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    d.drawLine(1, 59, 126, 59, SSD1306_WHITE);
    // 信箱、朴素时钟、窗户、左书桌、中间瓶子、右下猫窝。
    d.drawRect(9, 9, 20, 13, SSD1306_WHITE);
    d.drawLine(9, 9, 19, 16, SSD1306_WHITE);
    d.drawLine(28, 9, 19, 16, SSD1306_WHITE);
    d.drawRect(43, 7, 38, 14, SSD1306_WHITE);
    d.setCursor(47, 10);
    d.printf("%02u:%02u", static_cast<unsigned>(minuteOfDay / 60), static_cast<unsigned>(minuteOfDay % 60));
    d.drawRect(95, 8, 27, 25, SSD1306_WHITE);
    d.drawLine(108, 8, 108, 32, SSD1306_WHITE);
    d.drawLine(95, 20, 121, 20, SSD1306_WHITE);
    d.drawRect(8, 43, 36, 4, SSD1306_WHITE);
    d.drawLine(12, 47, 12, 57, SSD1306_WHITE);
    d.drawLine(39, 47, 39, 57, SSD1306_WHITE);
    d.drawLine(17, 40, 25, 42, SSD1306_WHITE);
    d.drawLine(25, 42, 33, 40, SSD1306_WHITE);
    d.drawLine(25, 37, 25, 42, SSD1306_WHITE);
    d.drawRoundRect(64, 37, 13, 19, 3, SSD1306_WHITE);
    d.drawRect(65, 33, 11, 4, SSD1306_WHITE);
    d.drawRoundRect(93, 54, 31, 5, 2, SSD1306_WHITE);
    // 静态自然蜷卧占位，不增加眨眼、舔毛或其他动画。
    d.drawEllipse(112, 49, 10, 6, SSD1306_WHITE);
    d.drawCircle(103, 45, 6, SSD1306_WHITE);
    d.drawTriangle(98, 42, 97, 36, 103, 40, SSD1306_WHITE);
    d.drawTriangle(105, 40, 110, 37, 109, 44, SSD1306_WHITE);
    d.drawPixel(101, 45, SSD1306_WHITE);
    d.drawPixel(105, 45, SSD1306_WHITE);
    d.drawPixel(103, 48, SSD1306_WHITE);
    if (m.selected == Selection::Desk) d.drawRect(5, 34, 42, 24, SSD1306_WHITE);
    if (m.selected == Selection::Stars) d.drawRect(60, 30, 21, 28, SSD1306_WHITE);
    if (m.selected == Selection::Mail) d.drawRect(6, 6, 26, 19, SSD1306_WHITE);
    if (m.selected == Selection::Clock) d.drawRect(40, 4, 44, 20, SSD1306_WHITE);
  } else {
    // 独立黑底入口占位，不伪造专注、奖励、信件或存档结果。
    d.setCursor(6, 5);
    d.print(m.pageName());
    if (m.page == Page::Desk) {
      d.drawRect(26, 31, 75, 5, SSD1306_WHITE);
      d.drawLine(33, 36, 33, 45, SSD1306_WHITE);
      d.drawLine(94, 36, 94, 45, SSD1306_WHITE);
    } else if (m.page == Page::Stars) {
      d.drawRoundRect(51, 23, 26, 25, 5, SSD1306_WHITE);
      d.drawRect(55, 19, 18, 4, SSD1306_WHITE);
    } else if (m.page == Page::Mail) {
      d.drawRect(38, 22, 52, 25, SSD1306_WHITE);
      d.drawLine(38, 22, 64, 36, SSD1306_WHITE);
      d.drawLine(89, 22, 64, 36, SSD1306_WHITE);
    } else {
      d.setTextSize(2);
      d.setCursor(33, 27);
      d.printf("%02u:%02u", static_cast<unsigned>(minuteOfDay / 60), static_cast<unsigned>(minuteOfDay % 60));
      d.setTextSize(1);
    }
    d.setCursor(6, 55);
    d.print("< BACK");
  }
  d.display();
}
