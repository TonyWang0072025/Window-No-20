# 固件开发与第一次串口测试

当前只有独立的 `serial_smoke` 测试程序，用于验证编译、烧录和真机运行。
它每秒输出一次运行时间，仅使用板载 USB 串口，不控制外接 OLED 或按钮，也不读写产品存档，不代表 P0 已完成。

## 已识别的原型硬件

| 项目 | 已知信息与证据 |
| --- | --- |
| 开发板 | 实物背面标记 NodeMCU ESP-32S V1.1，38 针，Type-C |
| 芯片 | esptool 实读 ESP32-D0WDQ6，revision v1.1，40 MHz 晶振 |
| Flash | esptool 实读 4 MB |
| USB 串口 | 实物 CP2102；Windows 硬件编号 `VID_10C4&PID_EA60` |
| Windows 驱动 | Silicon Labs 11.5.0.417，来自微软更新目录，目录签名验证通过 |
| 已观察端口 | COM5；重新插拔或换电脑后需重新确认 |
| 面包板 | MB-102；用户报告开发板已插好，后续接线前保持断电 |
| OLED | 实物四针标记依次 GND / VDD / SCK / SDA；驱动、分辨率和供电规格待核实 |

商品图的 CH340 标记与实物不符，按实物使用 CP210x 驱动。
硬件通信记录不包含设备 MAC 地址或其他个人标识。

## 工程选择与版本

- PlatformIO Core **6.2.0**、Espressif 32 平台 **8.0.0**：按固定版本管理构建与烧录依赖。
- Arduino-ESP32 **3.3.12**：由上述平台提供；后续 ESP32-C3 可继续使用同一开发框架。
- 板型：**ESP32 Dev Module**，PlatformIO ID `esp32dev`；Flash 4 MB，上传前核实实际串口。
- 串口输出：**115200 baud**。
- 本测试无需额外 Arduino 库。独立诊断工具 esptool **5.6.0** 用于之前的芯片信息读取。

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
