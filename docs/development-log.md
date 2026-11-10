# 原型开发验证记录

## 2026-11-10：硬件识别与首次烧录准备

基线：`main` 提交 `b9c0612c6be0ee5bb09589c2e214004655081db4`。
开发分支：`codex/sprint-01-prototype`。已确认的产品规则未修改。

| 检查 | 状态 | 证据 / 说明 |
| --- | --- | --- |
| 开发板实物识别 | 通过 | 用户提供正反面照片，NodeMCU ESP-32S V1.1、CP2102、Type-C、38 针 |
| USB 数据通信 | 通过 | Windows 检出 `VID_10C4&PID_EA60`；连接线支持数据 |
| CP210x 驱动 | 通过 | 问题代码从 28 变为 0，设备状态 OK，分配 COM5 |
| 烧录协议连接 / Flash 识别 | 通过 | esptool 5.6.0 的 `--port COM5 --baud 115200 flash-id` 返回 0，读取 ESP32-D0WDQ6 v1.1 / 4 MB，自动 RTS 复位完成；没有向 Flash 写入项目程序 |
| 面包板安装 | 用户已反馈 | 已指导断电安装至 b / i 列、第 1–19 行；用户回复“插好了”，不是电气测量结果 |
| OLED 识别 | 部分完成 | 四针 GND / VDD / SCK / SDA 已由照片确认；具体驱动、分辨率与供电规格未确认 |
| `serial_smoke` 编译 | 通过 | PlatformIO 6.2.0 / Espressif 32 8.0.0 / Arduino-ESP32 3.3.12；最终构建返回 0、显示 SUCCESS，无编译警告；程序 288212 / 1310720 字节，RAM 22116 / 327680 字节 |
| `serial_smoke` 烧录与运行 | 未测 | 等待重新连接开发板，再执行烧录并读取串口 |
| P0 OLED 与三键 | 未测 | 未接线、未编译相关功能、未在真机运行 |
| P1–P3 与正式低功耗 | 未测 | 尚未实现 |

### 构建复现与证据

- 从仓库根目录运行 `python -m platformio run --project-dir firmware --environment serial_smoke`；
  本机专用 Python 环境和 D 盘依赖目录的命令见 [固件说明](../firmware/README.md)。
- 实际包版本：`framework-arduinoespressif32` 4.30312.261024（Arduino 3.3.12），
  ESP32 运行库 3.3.12，Xtensa 工具链 14.2.0+20260121，烧录工具 esptool 5.4.0。
- 最终编译日志保存在本机 `D:\Window-No-20-Tools\serial-smoke-compile.log`；
  固件在 `D:\Window-No-20-Tools\platformio-build\serial_smoke\firmware.bin`，未将二进制或依赖提交到仓库。
- 固件 SHA-256：`c39f1c3520ec73ab196cdc902e11089a03c8005243b479d4a18dbbb21c88d96c`。
- 1310720 字节是当前应用分区上限，不是整块 4 MB Flash 的容量；以上占用也不代表完整产品占用。
- 尚未执行烧录或串口运行验收；没有以桌面单元测试代替真机结果。

### 本机安装过程中的已解决问题

- Silicon Labs 下载服务器拒绝直接下载，改从微软更新目录取得匹配
  `VID_10C4&PID_EA60` 的 11.5.0.417 驱动；安装前校验目录签名有效、签名人为
  Microsoft Windows Hardware Compatibility Publisher，安装后确认设备问题代码为 0。
- Python 工具下载受本机 HTTPS 代理协议配置影响；仅在安装子进程内改用 HTTP 代理连接，
  安装在项目专用虚拟环境，未修改系统代理或全局 Python 环境。

### 下一次真机操作

只连接开发板 USB，不接 OLED、按钮、电池或面包板电源模块。
先确认端口，再烧录串口测试程序，最后读取真实运行输出。
首次烧录成功不等同于 P0 验收通过。
