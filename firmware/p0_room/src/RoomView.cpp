#include "RoomView.h"
#include "ChineseGlyphs.h"

// 文案均为内部固定的三字节UTF-8汉字；只保留本原型需要的29个字。
static void chinese(Adafruit_SSD1306 &d, int x, int y, const char *text) {
  const auto *p = reinterpret_cast<const uint8_t *>(text);
  while (*p) {
    const uint16_t code = ((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
    for (const auto &glyph : chineseGlyphs) {
      if (glyph.code == code) { d.drawBitmap(x, y, glyph.pixels, 12, 12, SSD1306_WHITE); break; }
    }
    x += 12;
    p += 3;
  }
}

void drawRoomView(Adafruit_SSD1306 &d, const RoomModel &m, uint16_t minuteOfDay) {
  d.clearDisplay();
  d.setTextColor(SSD1306_WHITE);
  d.setTextSize(1);
  d.setTextWrap(false);
  const auto focusState = m.focus.state();
  if (m.page == Page::Desk && focusState != FocusState::Running) {
    // 先清空整帧，再画专属文字；暂停/完成绝不借用房间画面。
    if (focusState == FocusState::Selecting) {
      chinese(d, 22, 4, "请选择专注时长");
      d.setTextSize(2);
      d.setCursor(39, 25);
      d.printf("%u", static_cast<unsigned>(m.focus.selectedMinutes()));
      chinese(d, 69, 28, "分钟");
      chinese(d, 22, 49, "确认后开始计时");
    } else if (focusState == FocusState::Paused) chinese(d, 52, 26, "暂停");
    else if (focusState == FocusState::Completed) chinese(d, 40, 26, "专注完成");
    else if (focusState == FocusState::ExitConfirm) {
      chinese(d, 40, 4, "提前结束");
      d.setCursor(25, 29); d.print('<');
      chinese(d, 40, 26, "返回取消");
      d.setCursor(25, 50); d.print('>');
      chinese(d, 40, 47, "确认结束");
    }
  } else if (m.page == Page::Room || (m.page == Page::Desk && focusState == FocusState::Running)) {
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
    // 专注时瓶子稍下移，完整留出中上部倒计时；不能用数字黑底擦掉瓶口。
    const bool focusing = m.page == Page::Desk;
    d.drawRoundRect(64, focusing ? 43 : 37, 13, focusing ? 14 : 19, 3, SSD1306_WHITE);
    d.drawRect(65, focusing ? 39 : 33, 11, 4, SSD1306_WHITE);
    d.drawRoundRect(93, 54, 31, 5, 2, SSD1306_WHITE);
    if (m.page == Page::Desk) {
      // 功能占位：猫自然坐在书桌旁、前爪搭向摊开的书；正式美术仍须重做。
      d.drawEllipse(48, 53, 7, 6, SSD1306_WHITE);
      d.drawCircle(41, 45, 5, SSD1306_WHITE);
      d.drawTriangle(36, 43, 36, 38, 40, 40, SSD1306_WHITE);
      d.drawTriangle(43, 40, 47, 38, 46, 44, SSD1306_WHITE);
      d.drawLine(41, 49, 33, 42, SSD1306_WHITE);
      d.drawPixel(39, 46, SSD1306_WHITE);
      d.drawPixel(43, 46, SSD1306_WHITE);
      d.fillRect(34, 22, 60, 16, SSD1306_BLACK);
      d.setTextSize(2);
      d.setCursor(34, 22);
      const auto seconds = m.focus.secondsRemaining();
      d.printf("%02u:%02u", static_cast<unsigned>(seconds / 60), static_cast<unsigned>(seconds % 60));
    } else {
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
    }
  } else {
    // 独立黑底入口占位，不伪造专注、奖励、信件或存档结果。
    d.setCursor(6, 5);
    d.print(m.page == Page::Stars ? "TEST RAM" : m.pageName());
    if (m.page == Page::Stars) {
      d.drawRoundRect(22, 23, 26, 25, 5, SSD1306_WHITE);
      d.drawRect(26, 19, 18, 4, SSD1306_WHITE);
      d.setCursor(62, 28); d.printf("%lu", static_cast<unsigned long>(m.focus.stars()));
      d.setCursor(62, 40); d.print("STARS");
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
