# P1 专注计时器实施计划

> **For agentic workers:** 使用 superpowers:executing-plans 在本会话逐项执行，代码与测试由 Codex 自主完成。

**Goal:** 在已验证的屏幕、三键与 P0 导航上接入 P1，供作者每次一个小步骤实物验证。

**Architecture:** 延续 `firmware/p0_room`，增加不依赖 Arduino 的专注状态机；RoomModel 负责入口和返回，RoomView 负责128×64显示，main只采样按键和64位单调时间。测试直接运行真实模型，不模拟 GPIO 或显示库。

**Tech Stack:** 现有 PlatformIO 6.2.0 / espressif32 8.0.0 / esp32dev / Arduino-ESP32 3.3.12 / SSD1306 2.5.17 / GFX 1.13.0；Windows MSVC 用于主机测试。

**Spec:** `docs/design-baseline.md` 第4节、`docs/interaction-flows.md` 第1/3节、`docs/codex-sprint-01.md` P1、`docs/NEXT-SESSION.md`。

## Global Constraints

- 保留 P21/P22 OLED、P25/P26/P27三键及现有地线；不使用争议CND。
- 15/20/30/45/60分钟，自然完成1/1/1/2/3星；暂停和退出确认不计时。
- 默认构建使用真实时间，不提供可误开的加速固件；自动测试直接传入可控单调时间。
- 暂停与完成页独立纯黑，显示“暂停”/“专注完成”；完成页不擅加“+N星”。
- P1累计只是明确标为 TEST/RAM 的临时奖励验证，不读写NVS、不宣称正式星星存档；P2仍是后续必做项。
- 美术仍为待重做占位，不新增动画；伸懒腰衔接仍待确认。
- 用户已授权自主执行代码、环境和测试，不增加计划审批或重复硬件验证步骤；不自动合并main。

## Review Focus

- 最后不足一秒暂停/恢复，不能因四舍五入多扣时间；测试以毫秒边界断言。
- 归零同刻收到按键，不能提前退出吞奖或直接跳过完成页；测试归零边界三键。
- 退出确认取消后恢复原状态，不能悄悄恢复暂停或扣掉等待时间；测试两条路径。
- 跨32位毫秒范围、长暂停及主循环延迟，不能溢出或累计漂移；测试64位时间跳变。
- 完成页长停留、重复按键、新会话及重启，不能重复奖励；测试连续会话与全新模型。

### Task 1: 纯状态机与主机测试

**Files:** `firmware/p0_room/src/FocusTimer.h`、`RoomModel.h`、`state_tests.cpp`、`firmware/p0_room/test/`、`scripts/test-p1.ps1`。

**Interfaces:** RoomModel `handle(Key, uint64_t nowMs)`、`tick(uint64_t nowMs)`；FocusTimer提供只读状态、选中分钟、剩余秒和临时星数。

- [x] 先写书桌确认开始行为测试，在旧代码上观察失败。
- [x] 增加五档与奖励、无限暂停、退出冻结/恢复/放弃、归零单次奖、重启不恢复会话、64位时基测试。
- [x] 实现模型；更新 P0 入口测试以区分已实现书桌与其他占位页。
- [x] 主机测试全部通过：NAV 33 / P1 HOST 149，提交模型与测试。

### Task 2: 界面与板端接入

**Files:** `RoomView.*`、`main.cpp`、必要的最小中文字形资源；沿用原platformio.ini。

**Interfaces:** 消费 Task 1 的模型；main提供 `esp_timer_get_time()/1000`，以模型变化触发绘图与结构化日志。

- [x] 接入五档选择、书桌猫占位和中上方倒计时；暂停/完成清屏后只画对应文字。
- [x] 加入明确退出文案；瓶子页显示 TEST RAM 与临时计数，避免冒充存档。
- [x] 主机测试与 ESP32 编译通过；真实SSD1306缓冲区自检通过，暂停/完成无残余房间像素。
- [x] COM5核对及最终写入哈希通过，受控复位记录NAV/VIEW/READY明确PASS；原始日志留仓库外。

### Task 3: 交接与审查

**Files:** 固件README、根README、`docs/NEXT-SESSION.md`、`docs/development-log.md`、`CHANGELOG.md`。

- [x] 记录构建/测试/上传的实际结果，区分自动验证与作者肉眼验收；更新后续分支说明。
- [x] 独立只读审查无Critical/Important；修正发现的倒计时挡瓶口，重新构建/上传和画面自检通过。
- [x] 独立分支通过草稿PR #4交付，未合并main；当前人工待验只安排进入选时页。
