# 三个实体按钮测试

v0.2独立硬件诊断，不实现房间菜单或产品功能。保持已验证的OLED接线：
SDA d1/c1→P21、SCK d2/c2→P22（蓝线）、VDD d3/c3→3V3、GND d4/c4→GND。
ESP32不插面包板，使用公对母线直接连接底部长针。

## 按钮接线

三只按钮方向相同，跨中央沟槽，分别占d/g列的15/17、25/27、35/37行，取对角两脚。
下表为最初逐个地针的接法，已停用争议CND；后续替代回路见表后说明：

| 杜邦线面包板端 | ESP32端 | 说明 |
| --- | --- | --- |
| c15 | 另一个空着的GND | 与按钮d15同组；不是电源正极 |
| h17 | P25 | 与按钮g17同组 |
| c25 | 另一个空闲GND | 与第二只按钮d25同组 |
| h27 | P26 | 与第二只按钮g27同组 |
| c35 | 另一个空闲GND | 与第三只按钮d35同组 |
| h37 | P27 | 与第三只按钮g37同组 |

以上最初接法未全部通过：P25/P26正常，接争议CND回路的P27无响应。
争议CND停止使用；第三键c35线已移到第二键腾出的已验证GND，作者确认P27恢复。
第二键c25线目前ESP32端悬空，不参与该次对照。
不要继续假定还有第四个已验证GND，也不能将两个插头挤到同一针脚。
用户报告5V与SD3之间的接地标记实际读为“CND”；历史照片的此位置被识别为GND，
但尚无万用表导通确认。保留此不确定性，不将按钮状态或空闲读值当作引脚身份的完整证明。
新增两键已用掉此前剩余的四根杜邦线，后续不能默认还有多余线材。

所有插拔先断USB。只接GPIO与GND，不接5V/3V3到按钮；代码使用内部上拉，按下为低电平。
之前的GPIO回路电阻不能留在屏幕信号线之间。

### 待验证的第二键共地支路

为适配三个已验证GND及现有线材，计划断USB后整根取下闲置的c25线，
将一只1kΩ电阻两脚分别插入b25和b35。b25同组接第二键d25，b35同组通过c35线接已验证GND。
其余接线不变。第二键于是为P26→按钮→1kΩ→GND，内部上拉与1kΩ构成按下低电平路径。
这个支路仅用于按键输入限流；OLED和主板供电地线保持直连。
不占新GPIO、不增加杜邦线，腾出的c25线留作备用。作者已完成该桥接并反馈实物按压正常。

## 程序与构建

- Arduino-ESP32 3.3.12、PlatformIO espressif32 8.0.0、esp32dev。
- SSD1306 2.5.17、GFX 1.13.0、BusIO 1.17.4，128×64、I²C100kHz、SDA21/SCL22。
- `firmware/shared/DebouncedButton.h`独立处理30ms稳定输入，单次按下只发一个事件，释放单独记录。
  不依赖GPIO、显示或产品状态；启动时已按住不会伪造一次按下计数。
- `src/debounce_selftest.cpp`在启动时对真实使用的输入逻辑执行18项检查：
  按下抖动、门槛时刻、持续按住、松开抖动、再次按下、时间回绕、启动已按住。
  v0.2增加三路状态独立性6项，共24项。
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

1. 启动日志应有`DEBOUNCE_SELFTEST PASS checks=24`、OLED应答及三条`BUTTON_READY`。
2. 不碰按钮，屏幕三行25 BACK、26 SWITCH、27 OK均显示R和计数0；串口三路raw=1、pressed=0。
3. 三只按钮逐只按一下松开：仅对应计数加1，其他两行保持不变，日志各出现一次PRESSED和RELEASED。
4. 按住约2秒：计数只加1，持续按住不重复；松开后再次按下才加1。

v0.1下第一只按钮的第3项作者已确认单次计数加1、松手显示RELEASED，并反馈多次及快速按压正常。
第4项按住2秒未单独记录量化操作，仅启动软件逻辑自检验证不重复触发。
v0.2三键各自的实物操作尚待验证；完整P0仍未验收。
各项当前实际结果见 `docs/development-log.md`。
