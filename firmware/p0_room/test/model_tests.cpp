#include <cstdio>
#include <initializer_list>
#include "../src/RoomModel.h"
#include "../../shared/DebouncedButton.h"

static unsigned checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
  std::printf("FAIL line=%d: %s\n", __LINE__, #condition); return 1; } } while (false)

bool runRoomTests();

int main() {
  CHECK(runRoomTests());
  // 删掉预设循环、把分钟当秒、错配奖励或在完成后再次加星都会被捕获。
  struct Preset { unsigned minutes; uint64_t milliseconds; unsigned reward; };
  const Preset presets[] = {{15, 900000, 1}, {20, 1200000, 1}, {30, 1800000, 1},
                            {45, 2700000, 2}, {60, 3600000, 3}};
  for (unsigned index = 0; index < 5; ++index) {
    RoomModel m;
    m.handle(Key::Switch, 0);
    m.handle(Key::Confirm, 0);
    CHECK(m.focus.state() == FocusState::Selecting);
    for (unsigned n = 0; n < index; ++n) CHECK(m.handle(Key::Switch, 0));
    CHECK(m.focus.selectedMinutes() == presets[index].minutes);
    CHECK(m.handle(Key::Confirm, 100));
    CHECK(m.focus.state() == FocusState::Running);
    CHECK(m.focus.secondsRemaining() == presets[index].milliseconds / 1000);
    m.tick(100 + presets[index].milliseconds - 1);
    CHECK(m.focus.state() == FocusState::Running && m.focus.stars() == 0);
    CHECK(m.focus.secondsRemaining() == 1);
    CHECK(m.tick(100 + presets[index].milliseconds));
    CHECK(m.focus.state() == FocusState::Completed && m.focus.secondsRemaining() == 0);
    CHECK(m.focus.stars() == presets[index].reward);
    for (unsigned n = 0; n < 100; ++n) m.tick(100000000000ULL + n);
    CHECK(m.focus.stars() == presets[index].reward);
    CHECK(!m.handle(Key::Back, 100000000100ULL));
    CHECK(!m.handle(Key::Switch, 100000000100ULL));
    CHECK(m.handle(Key::Confirm, 100000000100ULL));
    CHECK(m.page == Page::Room && m.selected == Selection::None);
    CHECK(!m.handle(Key::Confirm, 100000000100ULL));
    // 新会话不得继承完成态；提前结束不能改变上一会话的奖星。
    m.handle(Key::Switch, 100000000200ULL);
    m.handle(Key::Confirm, 100000000200ULL);
    m.handle(Key::Confirm, 100000000200ULL);
    m.handle(Key::Back, 100000000300ULL);
    m.handle(Key::Confirm, 100000000400ULL);
    CHECK(m.page == Page::Room && m.focus.state() == FocusState::Abandoned);
    CHECK(m.focus.stars() == presets[index].reward);
    m.handle(Key::Switch, 100000001000ULL);
    m.handle(Key::Confirm, 100000001000ULL);
    m.handle(Key::Confirm, 100000001000ULL);
    m.tick(100000901000ULL);
    CHECK(m.focus.stars() == presets[index].reward + 1);
  }

  RoomModel cycle;
  cycle.handle(Key::Switch, 0); cycle.handle(Key::Confirm, 0);
  for (const unsigned minute : {20U, 30U, 45U, 60U, 15U}) {
    CHECK(cycle.handle(Key::Switch, 0));
    CHECK(cycle.focus.selectedMinutes() == minute);
  }
  CHECK(cycle.handle(Key::Back, 0));
  CHECK(cycle.page == Page::Room && cycle.focus.stars() == 0);

  // 暂停保留999ms以下的小数余量；跨32位计时范围且暂停数年仍不推进。
  RoomModel paused;
  paused.handle(Key::Switch, 0); paused.handle(Key::Confirm, 0);
  paused.handle(Key::Confirm, 4294967000ULL);
  paused.handle(Key::Confirm, 4294968501ULL); // 有效1501ms，剩898499ms。
  CHECK(paused.focus.state() == FocusState::Paused && paused.focus.secondsRemaining() == 899);
  CHECK(!paused.tick(200000000000ULL));
  CHECK(!paused.handle(Key::Switch, 200000000000ULL));
  paused.handle(Key::Back, 200000000000ULL);
  CHECK(paused.focus.state() == FocusState::ExitConfirm);
  paused.tick(300000000000ULL);
  paused.handle(Key::Back, 300000000000ULL);
  CHECK(paused.focus.state() == FocusState::Paused && paused.focus.secondsRemaining() == 899);
  paused.handle(Key::Confirm, 400000000000ULL);
  paused.tick(400000898498ULL);
  CHECK(paused.focus.state() == FocusState::Running && paused.focus.secondsRemaining() == 1);
  paused.tick(400000898499ULL);
  CHECK(paused.focus.state() == FocusState::Completed && paused.focus.stars() == 1);

  // 进行中进入退出页，取消只补回原状态；等待不扣秒。
  RoomModel exiting;
  exiting.handle(Key::Switch, 0); exiting.handle(Key::Confirm, 0); exiting.handle(Key::Confirm, 0);
  exiting.handle(Key::Back, 1250);
  CHECK(exiting.focus.state() == FocusState::ExitConfirm && exiting.focus.secondsRemaining() == 899);
  CHECK(!exiting.handle(Key::Switch, 10000000));
  CHECK(!exiting.tick(10000000));
  exiting.handle(Key::Back, 10000000);
  CHECK(exiting.focus.state() == FocusState::Running && exiting.focus.secondsRemaining() == 899);
  exiting.tick(10898749);
  CHECK(exiting.focus.state() == FocusState::Running && exiting.focus.secondsRemaining() == 1);
  exiting.handle(Key::Back, 10898749);
  exiting.handle(Key::Confirm, 20000000);
  CHECK(exiting.page == Page::Room && exiting.focus.stars() == 0);
  exiting.tick(30000000);
  CHECK(exiting.focus.stars() == 0);

  // 归零同刻的任一键先结算，完成画面需下一次确认才离开。
  for (const auto key : {Key::Back, Key::Switch, Key::Confirm}) {
    RoomModel boundary;
    boundary.handle(Key::Switch, 0); boundary.handle(Key::Confirm, 0); boundary.handle(Key::Confirm, 0);
    boundary.handle(key, 900000);
    CHECK(boundary.page == Page::Desk && boundary.focus.state() == FocusState::Completed);
    CHECK(boundary.focus.stars() == 1);
  }
  RoomModel delayed;
  delayed.handle(Key::Switch, 0); delayed.handle(Key::Confirm, 0); delayed.handle(Key::Confirm, 0);
  delayed.tick(6000000); // 主循环长延迟，仍只发本次一份奖励。
  CHECK(delayed.focus.state() == FocusState::Completed && delayed.focus.stars() == 1);
  RoomModel restarted;
  CHECK(restarted.page == Page::Room && restarted.selected == Selection::None);
  CHECK(restarted.focus.state() == FocusState::Idle && restarted.focus.stars() == 0); // P1仅RAM。

  // 直接运行真实去抖器：低电平保持只产生一次按下，松开后才可再次触发。
  DebouncedButton button;
  button.begin(false, 0);
  CHECK(button.update(true, 10) == ButtonEdge::None);
  CHECK(button.update(true, 39) == ButtonEdge::None);
  CHECK(button.update(true, 40) == ButtonEdge::Pressed);
  CHECK(button.update(true, 5000) == ButtonEdge::None);
  CHECK(button.update(false, 5010) == ButtonEdge::None);
  CHECK(button.update(false, 5040) == ButtonEdge::Released);
  CHECK(button.update(true, 5050) == ButtonEdge::None);
  CHECK(button.update(true, 5080) == ButtonEdge::Pressed);
  std::printf("P1_HOST_TESTS PASS checks=%u\n", checks);
  return 0;
}
