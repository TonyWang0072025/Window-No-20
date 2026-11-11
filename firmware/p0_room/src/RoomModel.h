#pragma once
#include <stdint.h>

enum class Key { Back, Switch, Confirm };
enum class Selection : int8_t { None = -1, Desk, Stars, Mail, Clock };
enum class Page { Room, Desk, Stars, Mail, Clock };

class RoomModel {
 public:
  Page page = Page::Room;
  Selection selected = Selection::None;

  bool handle(Key key) {
    if (key == Key::Back) {
      if (page == Page::Room && selected == Selection::None) return false;
      page = Page::Room;
      selected = Selection::None;
      return true;
    }
    if (page != Page::Room) return false;  // 子页目前仅验证进入/返回。
    if (key == Key::Switch) {
      selected = static_cast<Selection>((static_cast<int>(selected) + 1) % 4);
      return true;
    }
    if (selected == Selection::None) return false;
    page = static_cast<Page>(static_cast<int>(selected) + 1);
    return true;
  }

  const char *selectionName() const {
    constexpr const char *names[] = {"NONE", "DESK", "STARS", "MAIL", "CLOCK"};
    return names[static_cast<int>(selected) + 1];
  }
  const char *pageName() const {
    constexpr const char *names[] = {"ROOM", "DESK", "STARS", "MAIL", "CLOCK"};
    return names[static_cast<int>(page)];
  }
};
