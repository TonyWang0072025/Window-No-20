# P0导航 + P1专注原型 v0.2

## 范围

保留128×64画布，显示线框房间、静态猫占位、书桌/星星瓶/信箱/时钟。
上电直接进入无高亮房间；这不是生日欢迎，未读写首次欢迎或信箱标记。
返回清高亮，切换从书桌开始并依次循环，确认进入高亮物件。
书桌已接入P1，其余信箱和时钟仍为占位功能；从功能页返回清除高亮。

**P1是软件候选，待作者逐步实物验收；奖励仅 TEST RAM，重启归零，不能当作正式星星存档。**
P2仍须接入NVS写入/读回验证、断电保持和一次性标记；不使用本版积累正式星星。
时钟基础显示使用64位单调时钟，自上电12:00开始按分钟走；尚无校准、昼夜、天气、调暗/熄屏。
专注时猫移到书桌旁作为静态占位；目前全部线框美术仍须重做，未实现翻页或伸懒腰动画。

## P1操作与规则

| 页面 | 返回 | 切换 | 确认 |
| --- | --- | --- | --- |
| 书桌选时 | 回无高亮房间 | 15→20→30→45→60→15分钟 | 开始真实计时 |
| 进行 | 退出确认并冻结 | 无动作 | 暂停 |
| 暂停 | 退出确认并冻结 | 无动作 | 恢复计时 |
| 退出确认 | 取消，恢复此前进行/暂停态 | 无动作 | 提前结束，无奖励 |
| 完成 | 无动作 | 无动作 | 回无高亮房间 |

自然归零在RAM中立即记1/1/1/2/3测试星，同会话只记一次。瓶子页显示`TEST RAM`和准确临时数量。
默认固件没有加速开关，使用真实分钟与秒；主机测试直接传入可控64位毫秒，不改变设备行为。
末尾不足一秒向上取整显示；暂停和退出确认保留内部毫秒余量。到期同刻按键先结算，完成页留到下一次确认。
书桌入口每次从15分钟开始，这是本轮工程默认；后续可按实物反馈调整，不宣称作者另行批准了预设记忆规则。
同轮同时按多键只采用第一条按下（返回→切换→确认优先），避免一帧执行多个相互冲突的动作。
暂停和完成页分别仅显示“暂停”/“专注完成”，背景全黑；退出页明确写“返回取消”与“确认结束”。

## 文件

- `src/RoomModel.h`：与GPIO/显示独立的导航及书桌入口。
- `src/FocusTimer.h`：独立单调时间状态机与RAM测试奖励。
- `src/state_tests.cpp`：启动运行33项导航检查；`test/model_tests.cpp`：主机P1与去抖检查。
- `src/RoomView.*`、`ChineseGlyphs.h`：房间、中文专注页与占位图形。
- `src/view_tests.cpp`：用真实SSD1306缓冲区检查房间/瓶口可见、暂停/完成字区外全黑，失败阻止进入原型。
- `src/main.cpp`：已验证的GPIO/I²C适配、按键采样、基本走时与日志。
- 复用 `firmware/shared/DebouncedButton.h` 的30ms去抖，按住不重复触发。

## 接线

ESP32不插面包板。OLED SDA d1/c1→P21黑，SCK d2/c2→P22蓝，VDD d3/c3→3V3灰，GND d4/c4→已验证GND紫。
三按钮取对角脚：

| 按钮 | 四脚位置 | 信号线 | 地连接 |
| --- | --- | --- | --- |
| 返回 | d15/d17/g15/g17 | h17→P25 | c15→已验证GND |
| 切换 | d25/d27/g25/g27 | h27→P26 | b25→1kΩ→b35 |
| 确认 | d35/d37/g35/g37 | h37→P27 | c35→已验证GND |

b35与c35同组；电阻只在按钮输入支路，不串屏幕/主板电源地。争议CND不使用。
c25旧地线已取下备用；不新增接线。上述共地支路已获作者实物反馈通过。
正式成品按键应从左到右返回/切换/确认；当前面包板按第一/第二/第三只区分。

## 构建与上传

平台espressif32 8.0.0、esp32dev、Arduino-ESP32 3.3.12，SSD1306 2.5.17、GFX 1.13.0、BusIO1.17.4。
本机使用上级README中的专用Python与工具目录；独立构建目录为D:\Window-No-20-Tools\platformio-build-p1。

```powershell
$projectPython = Join-Path $env:LOCALAPPDATA 'Window-No-20/tools/python-env/Scripts/python.exe'
$env:PLATFORMIO_CORE_DIR = 'D:/Window-No-20-Tools/platformio'
$env:PLATFORMIO_BUILD_DIR = 'D:/Window-No-20-Tools/platformio-build-p1'
& $projectPython -m platformio run --project-dir firmware/p0_room --environment p0_room
& $projectPython -m serial.tools.list_ports
# 先核对端口；本机目前COM5。仅适用于当前已有正确引导程序/分区的设备，不适用于空白板。
& $projectPython -m esptool --chip esp32 --port COM5 --baud 115200 --after hard-reset write-flash --compress --flash-size 4MB 0x10000 D:/Window-No-20-Tools/platformio-build-p1/p0_room/firmware.bin
```

上传命令只对应当前已核对的分区与镜像。未整片擦除，不写NVS或eFuse。
本次实际成功使用esptool5.6.0默认stub、115200。此前ROM38400/19200写入分别发生0105格式错误，
降速未解决；换下载路径后成功。这个前后对比不证明底层通信问题已彻底修复。

## 验证

从仓库根目录运行`./scripts/test-p1.ps1`，使用本机已安装Visual Studio C++，构建产物留在仓库外。
预期`NAV_SELFTEST PASS checks=33`与`P1_HOST_TESTS PASS checks=149`。
覆盖A04–A10的模型行为及A11刷新不重复奖子项，另含64位时基、长暂停、毫秒余量、连续会话、重启取消与真实去抖器。
启动串口应有`NAV_SELFTEST PASS`、OLED应答、`P1_VIEW_SELFTEST PASS`及`P1_READY selftests=PASS`。
后续每秒`P1_STATE selftests=PASS ...`只在启动自检已通过时输出；测试使用独立模型，不给实际会话种入测试星。
这些自动检查不替代A04–A10实际屏幕/按键/走时的作者验收，也不证明A11重启保持或A15断电存档。

人工验收按顺序推进，每次只交办当前一项，**不要一次要求作者做完整表格**：

1. 当前只进入书桌选时页，观察“请选择专注时长”和15分钟。
2. 待上项反馈后，再逐项验证预设循环、开始递减、暂停/恢复、退出取消/结束。
3. 最后安排一段真实15分钟自然完成；其余四档奖励已由可控时间测试，不用让作者连续等所有时长。
4. P2实现后另做断电保存与防重复奖验收；当前禁止标记为通过。

接收曾出现异常重复字节，计数或时间明显不合理的采样不用于判定。
实际编译/烧录/设备自动检查/人工反馈分别记入docs/development-log.md。
