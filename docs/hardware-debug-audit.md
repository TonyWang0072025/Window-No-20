# 2026-11-11：OLED 排查方案复核

## 当前结论

根因未定位，OLED 未点亮；不能宣称 ESP32、OLED 或接线已经通过完整硬件验收。
停止重复恢复四线、逐根拔线、再恢复四线的流程。

当前板内已换成 `firmware/serial_smoke` 纯串口诊断程序，**不是 OLED 程序**。
它不包含屏幕库，不初始化 I²C，不主动驱动屏幕；当前不能期待屏幕出现测试画面。
四根线按用户最近确认保持连接，本轮未要求用户改变接线。

## 方案中的问题与纠正

- OLED 原程序只在 `setup()` 探测一次地址。之后每秒重复的 NO_ACK 是之前结果，
  不能当作每秒重新测量；每轮应区分启动探测与持续运行日志。
- 有些原始串口采样包含大量重复尾字符。一次 8.03 秒采样得到 1799380 字节，
  超出 115200、8N1 下正常 UART 接收量级，不能据此统计真实重启次数。
  换用 .NET 块接收后也曾出现重复数据，故不能简单归因于 Python 的逐行读取。
  已保存原始字节；重复来源仍未知。只取结构完整的复位记录作为启动异常证据。
- 先前把一次烧录 MD5 报错及 stub 读回不一致过早用于推断 Flash 内容损坏。
  本轮同一引导区在默认 stub 路径报错后，直接 ROM 校验却通过，结果不一致；
  需要区分通信/下载路径、内容完整性和真实运行，不能直接判断芯片损坏。
- 孔位表依赖开发板确实位于 b/i 列、第 1～19 行。背面丝印确认了引脚顺序，
  但照片与文字反馈不能替代当前四线连接的导通和电压测量。

## 本轮实际检查

| 检查 | 结果 | 能说明什么 |
| --- | --- | --- |
| Git 默认分支与 main | 远端 main 仍为 b9c0612c6be0ee5bb09589c2e214004655081db4；本地独立分支 | 产品基线未变，未推送或合并 |
| OLED 源码与板型配置 | SDA21/SCL22、100kHz，esp32dev、DIO40MHz、4MB；源码未改 | 没有发现把屏幕接在 Flash 管脚上的软件配置，但实际接线仍需核对 |
| 原 OLED 引导区、分区、应用 ROM 校验 | esptool 5.6.0、115200、`--no-stub`，三项通过 | 当次校验内容与构建镜像一致；不是全面存储可靠性测试 |
| 独立 .NET 串口块接收 | 捕获完整 RTCWDT_RTC_RESET 启动记录，未见应用标记 | 启动存在异常；不能仅由该原因码判断硬件根因 |
| 纯串口程序编译 | 成功，程序 288212/1310720 字节，RAM22116/327680 | 编译通过，不代表真机运行通过 |
| 默认 stub 上传纯串口程序 | 引导区 MD5 失败，应用尚未写入 | 与 OLED 绘图逻辑执行无关；报错值与第一次相同 |
| 同一引导区 ROM 复核 | 通过 | 与上一条 stub 路径结果不一致，不能直接认定引导区损坏 |
| ROM 115200、不压缩写入应用 | seq0 返回0105格式错误，失败 | 下载通信存在异常，该次不能作为成功写入；擦除之后需恢复应用 |
| ROM 38400、压缩写入纯串口应用 | 成功，292832字节镜像，写后哈希通过 | 已恢复有效应用；本轮同时改了速度与压缩方式，不能把成功单独归功于某一项 |
| 复位后再次 ROM 核对三项 | 引导、分区、纯串口应用全部通过 | 当前板内完整性校验通过 |
| 纯串口应用实际运行 | 仍捕获 RTCWDT_RTC_RESET，未见 RUNNING | 异常可在不包含 OLED 库、没有 I²C 调用的程序上出现；启动/框架、供电与接线因素仍需区分 |

本轮没有测量供电电压、电流、引脚电平或导通，没有测试 OLED 的独立应答。
能够进入芯片 ROM 并校验/写入程序，说明设备并非完全失去通信功能；
不能由此排除局部硬件损坏，也不能据此宣布屏幕损坏。

## 下一步的判别目标

用户确认没有万用表。下一步先在断 USB、保留现有四线的状态下，
核对当前完整接线照片中的实际孔位、线序及可能的相邻接触。
需要同时看清数字/列字母和 OLED 引脚丝印，不再让用户重复同一组拔线复现操作。
若该步骤仍不能区分硬件原因，再明确说明缺少什么测量证据，选择一次有判别力的检查。

## 本机证据与复现

原始日志在 `D:\Window-No-20-Tools`，未提交含设备标识的完整下载日志。

- `oled-audit-rom-verify.log`
- `oled-native-dotnet-all-four.bin`、`oled-native-after-rom-verify.bin`
- `audit-serial-build.log`、`audit-serial-upload.log`
- `audit-failed-upload-rom-verify.log`
- `audit-serial-rom-upload.log`、`audit-serial-rom-slow-upload.log`
- `audit-serial-slow-runtime.bin`、`audit-final-serial-rom-verify.log`

纯串口工程构建方法见 `firmware/README.md`。本轮独立构建目录为
`D:\Window-No-20-Tools\platformio-build-audit`，保留原 OLED 构建产物。
恢复应用使用：

```powershell
python -m esptool --chip esp32 --port COM5 --baud 38400 --no-stub --after hard-reset write-flash --compress --flash-size 4MB 0x10000 D:/Window-No-20-Tools/platformio-build-audit/serial_smoke/firmware.bin
```

该命令只对应本机已经核实的现有分区与构建文件，不是其他设备的通用烧录命令。
没有整片擦除，没有写 eFuse，没有停用看门狗，没有更改产品规则。

官方依据：
[RTC 看门狗与启动阶段](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/fatal-errors.html)、
[绕过 stub 的含义](https://docs.espressif.com/projects/esptool/en/latest/esp32/esptool/advanced-options.html)、
[烧录排查](https://docs.espressif.com/projects/esptool/en/latest/esp32/troubleshooting.html)。
