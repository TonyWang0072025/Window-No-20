#include <Arduino.h>
#include "RoomModel.h"

bool runRoomTests() {
  unsigned checks = 0;
  auto check = [&checks](bool ok) { ++checks; return ok; };
  RoomModel m;
  if (!check(m.page == Page::Room && m.selected == Selection::None) ||
      !check(!m.handle(Key::Back)) || !check(!m.handle(Key::Confirm))) return false;
  const Selection cycle[] = {Selection::Desk, Selection::Stars, Selection::Mail,
                             Selection::Clock, Selection::Desk};
  for (auto target : cycle) {
    if (!check(m.handle(Key::Switch) && m.selected == target && m.page == Page::Room)) return false;
  }
  if (!check(m.handle(Key::Back) && m.selected == Selection::None)) return false;
  for (int target = 0; target < 4; ++target) {
    RoomModel item;
    for (int step = 0; step <= target; ++step) item.handle(Key::Switch);
    if (!check(item.handle(Key::Confirm) && item.page == static_cast<Page>(target + 1)) ||
        !check(!item.handle(Key::Switch)) || !check(!item.handle(Key::Confirm)) ||
        !check(item.handle(Key::Back) && item.page == Page::Room && item.selected == Selection::None) ||
        !check(!item.handle(Key::Confirm)) ||
        !check(item.handle(Key::Switch) && item.selected == Selection::Desk)) return false;
  }
  Serial.printf("P0_SELFTEST PASS checks=%u\n", checks);
  return true;
}
