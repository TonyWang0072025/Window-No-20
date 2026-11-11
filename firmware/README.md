# 固件开发与硬件诊断

`serial_smoke` 是独立串口测试程序，用于验证编译、烧录和真机运行。
它每秒输出一次运行时间，仅使用板载 USB 串口，不控制外接 OLED 或按钮，也不读写产品存档。
`oled_smoke` 是单独的首次点亮工程，检测 I2C 应答并尝试显示 128×64 测试画面。
`gpio_loopback` 是独立信号线回路诊断，正对照尚未进行。
`button_smoke` 保留OLED，v0.2分别测试P25/P26/P27的三只按钮，见[按钮诊断指南](button_smoke/README.md)。
`p0_room` 实现线框房间和四入口的三键导航，见[P0原型指南](p0_room/README.md)。
**当前板内为p0_room**；自动导航检查已运行，实际显示/操作等待作者逐步验收。诊断程序不代表完整产品已完成。

## 已识别的原型硬件

| 项目 | 已知信息与证据 |
| --- | --- |
| 开发板 | 实物背面标记 NodeMCU ESP-32S V1.1，38 针，Type-C |
| 芯片 | esptool 实读 ESP32-D0WDQ6，revision v1.1，40 MHz 晶振 |
| Flash | esptool 实读 4 MB |
| USB 串口 | 实物 CP2102；Windows 硬件编号 `VID_10C4&PID_EA60` |
| Windows 驱动 | Silicon Labs 11.5.0.417，来自微软更新目录，目录签名验证通过 |
| 已观察端口 | COM5；重新插拔或换电脑后需重新确认 |
| 面包板 | MB-102；当前OLED在d1～d4、ESP32独立放置，接线前保持断电 |
| OLED | GND / VDD / SCK / SDA四针；已在3.3V、SSD1306兼容128×64配置下实测正常显示，地址0x3C；具体芯片身份未确认 |

OLED 配套教程的接线图使用 3V3，示例使用 SSD1306 驱动；下载包中 Python 示例为128×64，
Arduino 示例仍为128×32，正文还有旧LCD参数。因此首轮测试采用3.3V和SSD1306兼容128×64配置，
是否适用于实际屏幕须以 I2C 应答和实际显示结果确认。
来源：[套件教程](https://doc.7gp.cn/esp32-kits/beginner/Arduino/arduino-tutorial/)、
[接线图](https://doc.7gp.cn/esp32-kits/media/229ce4e1ad8b01c71577d8f777e8195b.png)、
[配套代码入口](https://doc.7gp.cn/esp32-kits/beginner/board/downloads/)。

商品图的 CH340 标记与实物不符，按实物使用 CP210x 驱动。
硬件通信记录不包含设备 MAC 地址或其他个人标识。

## 工程选择与版本

- PlatformIO Core **6.2.0**、Espressif 32 平台 **8.0.0**：按固定版本管理构建与烧录依赖。
- Arduino-ESP32 **3.3.12**：由上述平台提供；后续 ESP32-C3 可继续使用同一开发框架。
- 板型：**ESP32 Dev Module**，PlatformIO ID `esp32dev`；Flash 4 MB，上传前核实实际串口。
- 串口输出：**115200 baud**。
- 串口测试无需额外 Arduino 库。独立诊断工具 esptool **5.6.0** 用于之前的芯片信息读取。

安装依据：[PlatformIO 平台 8.0.0 发布说明](https://github.com/platformio/platform-espressif32/releases/tag/v8.0.0)、[ESP32 Dev Module 配置](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html)。

## 通用构建命令

以下命令从仓库根目录运行。首次安装需要联网下载免费工具；不要求设备联网。

```powershell
python -m pip install platformio==6.2.0
python -m platformio run --project-dir firmware --environment serial_smoke
```

本机采用项目专用 Python 环境，未修改系统 PATH；实际运行时使用：

```powershell
$toolPython = "$env:LOCALAPPDATA\Window-No-20\tools\python-env\Scripts\python.exe"
$env:PLATFORMIO_CORE_DIR = 'D:\Window-No-20-Tools\platformio'
$env:PLATFORMIO_BUILD_DIR = 'D:\Window-No-20-Tools\platformio-build'
& $toolPython -m platformio run --project-dir firmware --environment serial_smoke
```

本机需要联网下载组件时，在该 PowerShell 子进程额外设置
`$env:HTTPS_PROXY = 'http://127.0.0.1:7897'`，对应当前本机代理配置；未修改系统代理。

本机大体积依赖和构建产物保存在仓库外的上述 D 盘目录。
其他电脑不设这两个环境变量也可构建，默认构建目录 `firmware/.pio/` 已加入忽略列表。

## 第一次烧录的连接与验收

本测试只接开发板 USB 至电脑，外设全部不接，电池和面包板电源模块不接。
在断电状态检查开发板两排针分处中央沟槽两侧，无散落导线、无相邻针短接；
面包板放在干燥非金属桌面，再连接 USB。板载电源灯亮只代表已通电。

先用 `python -m platformio device list` 或 Windows 设备管理器确认端口，再执行：

```powershell
python -m platformio run --project-dir firmware --environment serial_smoke --target upload --upload-port COM5
python -m platformio device monitor --port COM5 --baud 115200
```

本机使用上面的 `$toolPython` 替代 `python`，并保留上述两个目录环境变量。
烧录完成后串口监视器可能错过启动首行，可按一次板载 EN 复位键重新观察。
不要同时打开两个串口监视器；测试结束关闭监视器以释放端口。

期望输出示例（示例，不是实测记录）：

```text
WINDOW_NO_20 SERIAL_SMOKE v0.1
chip=ESP32-D0WDQ6 revision=101 flash_bytes=4194304
RUNNING uptime_ms=300
RUNNING uptime_ms=1300
```

验收：烧录工具报告成功，并在真实开发板上连续读到至少五条 `RUNNING`，
运行时间每条相隔约 1000 ms；没有反复重启。芯片名和修订号的输出形式以框架实际返回为准。
编译结果与真机结果分别记在 [开发验证记录](../docs/development-log.md)。

本机已观察到 `readline()` / `.NET ReadLine()` 返回重复时间值，底层原因尚未定位。
本次实际验收采用 pySerial `read(2048)`、7 秒超时接收字节块后按换行解析，
连续收到7条间隔1000毫秒的真实记录；详见上述验证记录。

## OLED 首次点亮工程

### 当前接线表

本表对应用户重新摆放后的布局：OLED四根针在d1～d4，ESP32取下面包板。
公对母杜邦线的插孔端直接套ESP32底部长针，金属针端插入c1～c4。
同一行c/d孔相通，各数字行互相独立。之前ESP32在b/i列、第1～19行的孔位表已停用。

| OLED丝印 | 当前线色 | 开发板连接 | OLED针脚孔 | 杜邦线金属针孔 |
| --- | --- | --- | --- | --- |
| GND | 紫 | GND | d4 | c4 |
| VDD | 灰 | 3V3 | d3 | c3 |
| SCK | 蓝（用户替换了白线） | GPIO22 / I2C SCL | d2 | c2 |
| SDA | 黑 | GPIO21 / I2C SDA | d1 | c1 |

接线时 USB 保持拔掉，电池和面包板电源模块不接；ESP32放在干燥非金属支撑物上。
OLED四针位置由用户照片与文字确认；蓝线替换及新接线由用户报告完成。
以实际引脚文字为准，不按常见电源线颜色习惯推断。GPIO回路测试电阻不接入此布局。

### 构建与烧录

此工程使用与串口工程相同的平台版本，另固定 Adafruit SSD1306 2.5.17、
Adafruit GFX Library 1.13.0；本次BusIO依赖实际解析为1.17.4。

```powershell
python -m platformio run --project-dir firmware/oled_smoke --environment oled_smoke
python -m platformio run --project-dir firmware/oled_smoke --environment oled_smoke --target upload --upload-port COM5
```

本机仍用上面的专用Python、D盘工具目录和下载代理设置。
程序在GPIO21/22上使用100kHz I2C，只探测SSD1306常用的0x3C/0x3D地址。
无应答或初始化失败时每秒输出明确错误，保留现场以排查接线。

### 验收与边界

- 串口出现 `I2C_PROBE ... status=0`，随后出现 `OLED_INIT ... canvas=128x64`，证明连接设备有应答、程序已初始化绘图缓冲。
- 实物应看到完整细边框、`WINDOW No.20`、`OLED 128x64`、I2C地址，以及每秒变化的秒数。
- 串口 `OLED_UPDATE` 仅表示调用了画面更新，不能单独证明屏幕亮起或所有像素正确；实际画面需用户观察确认。
- 未连接三个按钮，未实现房间、产品状态机、中文字体或存档；不改产品规则。
