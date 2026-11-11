# 第一只实体按钮测试

独立硬件诊断，不实现房间菜单或产品功能。保持已验证的OLED接线：
SDA d1/c1→P21、SCK d2/c2→P22（蓝线）、VDD d3/c3→3V3、GND d4/c4→GND。
ESP32不插面包板，使用公对母线直接连接底部长针。

## 按钮接线

作者确认四脚在d15、d17、g15、g17，取对角两脚：

| 杜邦线面包板端 | ESP32端 | 说明 |
| --- | --- | --- |
| c15 | 另一个空着的GND | 与按钮d15同组；不是电源正极 |
| h17 | P25 | 与按钮g17同组 |

所有插拔先断USB。只接GPIO与GND，不接5V/3V3到按钮；代码使用内部上拉，按下为低电平。
无需接1kΩ电阻；之前的GPIO回路电阻不能留在屏幕信号线之间。

## 程序与构建

- Arduino-ESP32 3.3.12、PlatformIO espressif32 8.0.0、esp32dev。
- SSD1306 2.5.17、GFX 1.13.0、BusIO 1.17.4，128×64、I²C100kHz、SDA21/SCL22。
- `firmware/shared/DebouncedButton.h`独立处理30ms稳定输入，单次按下只发一个事件，释放单独记录。
  不依赖GPIO、显示或产品状态；启动时已按住不会伪造一次按下计数。
- `src/debounce_selftest.cpp`在启动时对真实使用的输入逻辑执行18项检查：
  按下抖动、门槛时刻、持续按住、松开抖动、再次按下、时间回绕、启动已按住。
  逻辑自检不能替代实物按钮验收；自检失败会明确报错并停止进入按钮测试。
- 画面仅在事件或每秒状态更新时刷新；目前沿用100kHz全帧传输，会暂时阻塞采样。
  作者已反馈多次及快速按压正常；连击间隔、次数未计量，最大响应频率尚未定量验证。

```powershell
python -m platformio run --project-dir firmware/button_smoke --environment button_smoke
```

本机使用专用Python和工具目录（见上级README），独立构建目录为
`D:\Window-No-20-Tools\platformio-build-button`。同一分区布局下只写应用：

```powershell
python -m esptool --chip esp32 --port COM5 --baud 38400 --no-stub --after hard-reset write-flash --compress --flash-size 4MB 0x10000 D:/Window-No-20-Tools/platformio-build-button/button_smoke/firmware.bin
```

## 验收方式

1. 启动日志应有`DEBOUNCE_SELFTEST PASS checks=18`、OLED应答及`BUTTON_READY`。
2. 不碰按钮，屏幕应显示`P25: RELEASED`、`PRESS COUNT: 0`；串口raw=1、pressed=0。
3. 按一下松开：计数只加1，日志各出现一次PRESSED和RELEASED。
4. 按住约2秒：计数只加1，持续按住不重复；松开后再次按下才加1。

第3项作者已确认单次计数加1、松手显示RELEASED，并反馈多次及快速按压正常。
第4项按住2秒未单独记录量化操作，仅启动软件逻辑自检验证不重复触发。
第二、第三只按钮及完整P0仍未验收。
各项当前实际结果见 `docs/development-log.md`。
