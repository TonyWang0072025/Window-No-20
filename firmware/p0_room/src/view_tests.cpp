#include <cstdio>
#include "RoomView.h"

// 使用真实SSD1306绘图缓冲区，不用假显示器；启动时运行，稍后重绘正常房间。
bool runRoomViewTests(Adafruit_SSD1306 &display) {
  RoomModel test;
  test.handle(Key::Switch, 0); test.handle(Key::Confirm, 0); test.handle(Key::Confirm, 0);
  drawRoomView(display, test, 720);
  if (!display.getPixel(0, 0) || !display.getPixel(127, 63) || !display.getPixel(69, 39)) return false;
  test.handle(Key::Confirm, 1234);
  for (unsigned frame = 0; frame < 2; ++frame) {
    drawRoomView(display, test, 720);
    unsigned lit = 0;
    for (int y = 0; y < 64; ++y) for (int x = 0; x < 128; ++x) {
      if (!display.getPixel(x, y)) continue;
      ++lit;
      const int left = frame == 0 ? 52 : 40;
      const int right = frame == 0 ? 76 : 88;
      if (x < left || x >= right || y < 26 || y >= 38) return false;
    }
    if (lit < 50) return false; // 防止空白屏冒充正确黑底文字。
    // 由暂停恢复，再自然归零；第二帧必须清掉全部房间像素。
    if (frame == 0) {
      test.handle(Key::Confirm, 1234);
      test.tick(900000);
    }
  }
  std::printf("P1_VIEW_SELFTEST PASS running_room=YES bottle_visible=YES paused_black=YES completed_black=YES\n");
  return true;
}
