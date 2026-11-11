# P0房间导航原型

## 范围

保留128×64画布，显示线框房间、静态猫占位、书桌/星星瓶/信箱/时钟。
上电直接进入无高亮房间；这不是生日欢迎，未读写首次欢迎或信箱标记。
返回清高亮，切换从书桌开始并依次循环，确认进入高亮物件的独立黑底占位页。
从占位页返回清除高亮，已由作者2026-11-11确认。

功能页目前只验证进入/返回，没有专注计时、奖励、真实星星数、信件或NVS。
时钟基础显示使用64位单调时钟，自上电12:00开始按分钟走；尚无校准、昼夜、天气、调暗/熄屏。
没有新增猫咪动作；目前是静态线框，不代表正式像素美术。

## 文件

- `src/RoomModel.h`：与GPIO/显示独立的导航规则。
- `src/state_tests.cpp`：启动运行33项导航状态检查，失败时禁止进入原型。
- `src/RoomView.*`：房间、描边与四个占位页绘制。
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
本机使用上级README中的专用Python与工具目录；独立构建目录为D:\Window-No-20-Tools\platformio-build-p0。

```powershell
python -m platformio run --project-dir firmware/p0_room --environment p0_room
python -m esptool --chip esp32 --port COM5 --baud 115200 --after hard-reset write-flash --compress --flash-size 4MB 0x10000 D:/Window-No-20-Tools/platformio-build-p0/p0_room/firmware.bin
```

上传命令只对应当前已核对的分区与镜像。未整片擦除，不写NVS或eFuse。
本次实际成功使用esptool5.6.0默认stub、115200。此前ROM38400/19200写入分别发生0105格式错误，
降速未解决；换下载路径后成功。这个前后对比不证明底层通信问题已彻底修复。

## 验证

自动检查覆盖初始无高亮、无选中时返回/确认无效、完整循环、各入口确认、子页按键不扩展功能、返回清高亮。
串口应有`P0_SELFTEST PASS checks=33`、OLED应答及`P0_READY page=ROOM selected=NONE`。
实际作者按键分步验证A01/A02/A03；自动检查不能替代实际显示与按钮观察。
接收曾出现异常重复字节，计数或时间明显不合理的采样不用于判定。
实际编译/烧录/设备自动检查/人工反馈分别记入docs/development-log.md。
