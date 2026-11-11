#include <cstdio>
#include "RoomModel.h"

bool runRoomTests() {
  unsigned checks = 0;
  auto check = [&checks](bool ok) { ++checks; return ok; };
  RoomModel m;
  if (!check(m.page == Page::Room && m.selected == Selection::None) ||
      !check(!m.handle(Key::Back, 0)) || !check(!m.handle(Key::Confirm, 0))) return false;
  const Selection cycle[] = {Selection::Desk, Selection::Stars, Selection::Mail,
                             Selection::Clock, Selection::Desk};
  for (auto target : cycle) {
    if (!check(m.handle(Key::Switch, 0) && m.selected == target && m.page == Page::Room)) return false;
  }
  if (!check(m.handle(Key::Back, 0) && m.selected == Selection::None)) return false;
  for (int target = 0; target < 4; ++target) {
    RoomModel item;
    for (int step = 0; step <= target; ++step) item.handle(Key::Switch, 0);
    if (!check(item.handle(Key::Confirm, 0) && item.page == static_cast<Page>(target + 1))) return false;
    if (target == 0) {
      if (!check(item.focus.state() == FocusState::Selecting) ||
          !check(item.handle(Key::Switch, 0) && item.focus.selectedMinutes() == 20)) return false;
    } else if (!check(!item.handle(Key::Switch, 0)) || !check(!item.handle(Key::Confirm, 0))) return false;
    if (!check(item.handle(Key::Back, 0) && item.page == Page::Room && item.selected == Selection::None) ||
        !check(!item.handle(Key::Confirm, 0)) ||
        !check(item.handle(Key::Switch, 0) && item.selected == Selection::Desk)) return false;
  }
  std::printf("NAV_SELFTEST PASS checks=%u\n", checks);
  return true;
}
