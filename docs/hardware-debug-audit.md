# 2026-11-11：OLED 排查方案复核

## 当前结论

**本文后续大部分内容是排查历史，不是当前接线指令。**当前板内为`firmware/p0_room`，
屏幕、三键和P0核心导航已获作者反馈通过。最新接线为OLED在面包板、ESP32独立放置，
第二键通过1kΩ共用第三键已验证地连接，争议CND停用。请先读`NEXT-SESSION.md`，
详细结果见`development-log.md`最新章节，不能照历史孔位表重接。
此前故障的唯一根因未定位；不能将一次显示通过当作整机、长期稳定性或P0完整交互验收。

## 最新布局与显示实测

- 作者将OLED安装到面包板d1～d4：SDA d1、SCK d2、VDD d3、GND d4；
  ESP32取下面包板，杜邦线插孔端直接套开发板底部长针。
- 对应连接为黑c1→P21、蓝c2→P22、灰c3→3V3、紫c4→GND。
  蓝线替换原白线。孔位照片与作者文字确认，接线表见 `firmware/README.md`。
- 恢复OLED固件前，GPIO自检程序在新布局下连续输出6条ALIVE，时间每次相隔1000ms。
- 恢复原OLED固件，SHA256仍为8ca72bf9e82a1285eb8b7c60a7672812f2b710fd31b333018a73a0d93d4c2f4d；
  使用esptool5.6.0、ROM、38400、压缩，仅写现有0x10000应用分区。360896字节，55.3秒，写后哈希通过，返回0。
- 首次运行采样8.02秒收到7条OLED_UPDATE，uptime_ms为29031/30131/31231/32331/33431/34531/35631。
- 作者反馈“文字、边框和秒数都正常显示”，实际显示验收通过。
- 随后一次受控复位采样8.11秒，启动输出：
  `I2C_PROBE address=0x3C status=0`、`OLED_INIT address=0x3C canvas=128x64`，
  随后7条更新，从432ms递增至7031ms，未见异常重启。
- 固件没有为此次点亮修改显示逻辑；安装方式、接触条件及SCK线均变化，
  支持继续采用新连接，但不能区分此前是哪个接点、哪根线或哪种电气因素导致故障。
  不将“此前完全没接上电源”或“OLED/ESP32曾损坏”记录为已证实原因。
- 本机证据：`oled-new-layout-before-upload-confirm.log`、`oled-new-layout-rom-upload.log`、
  `oled-new-layout-runtime.bin/.log`、`oled-new-layout-startup.bin/.log`。

## 完全断开屏幕后与GPIO回路检查

- OLED端四根全部断开、面包板端不动时，纯串口程序8.02秒收到8条RUNNING，
  uptime从52317到59317，间隔均1000ms。初次采样有重复数据，不作通过依据。
- 加入 `firmware/gpio_loopback` 独立诊断：无OLED/I²C库，GPIO21/22仅开漏拉低或释放，
  内部弱上拉，串口T命令触发两路高低电平跟随检查。
- 编译通过，无编译警告，程序291036/1310720字节、RAM22124/327680；
  ROM38400压缩上传295584字节镜像，46.8秒，哈希通过。
- 无回路电阻的负对照采样6.02秒，四项读值符合预期：21拉低为(0,1)，释放为(1,1)，
  22拉低为(1,0)，释放为(1,1)，最终`passed=2 total=4 result=FAIL`；随后6条ALIVE正常递增。
  这说明未连接回路时不会误判连通，不能证明外部信号线导通。
- 作者找到1kΩ电阻但尚未接入，即改用OLED装在面包板、ESP32独立放置的新方案；
  电阻正对照**未测**。当前已恢复OLED固件，不能对它发送GPIO自检命令。
- 本机证据：`audit-serial-oled-disconnected-confirm.bin/.log`、
  `gpio-loopback-build.log`、`gpio-loopback-rom-upload.log`、`gpio-loopback-open-negative-confirm.bin/.log`。

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

## 点亮前的实际检查（历史排查结果）

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

## 点亮前后续排查记录（已结束）

用户确认没有万用表。随后提供四张断电接线照片，包括 OLED 正面丝印、
屏幕端线色及面包板两侧连接。可辨认的线序与落孔符合当前表：
GND紫j2、VDD灰j19、SCK白a17、SDA黑a14；没有发现明确反接或错行证据。
这不是导通、电压或接触可靠性测量，不能由照片排除线材和硬件内部问题。

用户已更换电脑端 USB 接口，四根屏幕线保持连接。COM5仍可识别。
先采样10秒未收到数据；随后受控复位、采样12.11秒，收到612字节，
包含一次 POWERON_RESET 启动记录及一次 RTCWDT_RTC_RESET，未见 RUNNING。
因此换接口后尚未观察到恢复，不能仅凭此排除 USB 线、供电或转换芯片问题。
证据：本机 `audit-other-usb-port-runtime.bin/.log`、`audit-other-usb-port-reset.bin/.log`。

下一次对照保持当前纯串口程序、USB接口及面包板落孔不变，断USB后在OLED端
整体取下四个插头，使屏幕完全脱离，再接USB观察基础程序是否进入RUNNING。
若仍失败，优先查开发板/基础启动链，不再围绕OLED逐根拔线；若恢复，才能把
当前故障范围收敛到屏幕接入造成的电气影响，仍不能直接判定OLED内部损坏。
此前隔离实验使用的是OLED程序，且存在异常接收数据，不能替代这次纯串口对照。

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
