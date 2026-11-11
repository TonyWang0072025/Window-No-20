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
| `serial_smoke` 烧录 | 通过 | COM5、115200 baud，PlatformIO 上传返回 0，所有写入段均报告 Hash of data verified，最终 SUCCESS |
| `serial_smoke` 真机运行 | 通过 | pySerial 按字节块接收，7.016 秒内取得 7 条 RUNNING，相邻运行时间均增加 1000 ms；本次采样期间未见重启 |
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
- 已执行真实 USB 烧录及串口运行验收；没有以桌面单元测试代替真机结果。

### 首次烧录与真实运行

- 上传前再次确认 CP2102 设备状态 OK、COM5，以及固件 SHA-256 与上述编译产物一致。
- 上传命令：`python -m platformio run --project-dir firmware --environment serial_smoke --target upload --upload-port COM5`。
- 上传日志保存在本机 `D:\Window-No-20-Tools\serial-smoke-upload.log`。
- 原始运行数据和采样结果分别保存在本机 `D:\Window-No-20-Tools\serial-smoke-runtime.log`
  与 `D:\Window-No-20-Tools\serial-smoke-runtime-result.json`。
- 接收方法：COM5、115200 baud，关闭 DTR / RTS 后打开端口、清空接收缓冲区，
  执行 `read(2048)`，超时 7 秒；关闭串口后按换行解析收到的字节块。

真实采样输出：

```text
RUNNING uptime_ms=235317
RUNNING uptime_ms=236317
RUNNING uptime_ms=237317
RUNNING uptime_ms=238317
RUNNING uptime_ms=239317
RUNNING uptime_ms=240317
RUNNING uptime_ms=241317
```

本次基础通信与运行验收通过，仍不代表长时间稳定性、OLED、按钮、产品功能或低功耗通过。

### 串口接收中观察到的限制

- 最初使用 Python `readline()`、随后使用 .NET `ReadLine()`，都读到了重复的同一条时间值；
  这两次逐行接收结果没有用于判定运行通过。
- 改为按字节块 `read(512)` 后，在三次独立接收中读到正常递增的运行时间；
  最终又以 `read(2048)` 连续采样 7 秒通过。固件代码在这期间没有修改。
- 逐行读取异常的底层原因尚未定位，未将其归因于具体驱动或固件；当前验证日志采用块接收。

### 本机安装过程中的已解决问题

- Silicon Labs 下载服务器拒绝直接下载，改从微软更新目录取得匹配
  `VID_10C4&PID_EA60` 的 11.5.0.417 驱动；安装前校验目录签名有效、签名人为
  Microsoft Windows Hardware Compatibility Publisher，安装后确认设备问题代码为 0。
- Python 工具下载受本机 HTTPS 代理协议配置影响；仅在安装子进程内改用 HTTP 代理连接，
  安装在项目专用虚拟环境，未修改系统代理或全局 Python 环境。
- 首次上传还需补装文件系统工具；未带上述代理设置的首次调用在补装阶段报 HTTPClientError，
  尚未开始写 Flash。为上传子进程应用同一代理设置后，补装、上传和校验成功。

### 下一次硬件准备

核实 OLED 的实际供电电压、驱动和分辨率，查阅套件配套资料后再给出接线表。
接外设之前先拔掉开发板 USB；电池和面包板电源模块继续不接。
首次烧录成功不等同于 P0 验收通过。

## 2026-11-11：OLED接线与首次点亮准备

- 用户提供套件教程入口 `https://doc.7gp.cn/esp32/`。已读取OLED章节和下载包中的Arduino/Python显示示例。
- 资料存在旧LCD与OLED混用：接线图接3V3，示例用SSD1306；Python为128×64，Arduino仍为128×32。
  首轮据此采用3.3V、128×64、SSD1306兼容配置进行验证，不将示例当作实物芯片身份的证明。
- OLED正面丝印与已插线照片交叉确认：GND紫、VDD灰、SCK白、SDA黑。
- 用户已报告紫色线插入j2，随后确认灰/白/黑三根已按j19 / a17 / a14接入，且报告这三根略松；未要求补拍照片。
  用户明确不需要逐根拍照，后续按文字确认推进；上电前仍需核对电源与线序。
- 新增独立 `firmware/oled_smoke` 诊断工程：检测I2C应答，显示边框、文字、地址与秒数；不实现产品UI。
- 编译：通过，返回0并显示SUCCESS，无编译警告；程序354868 / 1310720字节，RAM 23676 / 327680字节。
- 实际库版本：Adafruit SSD1306 2.5.17、Adafruit GFX Library 1.13.0、Adafruit BusIO 1.17.4。
- 编译命令：`python -m platformio run --project-dir firmware/oled_smoke --environment oled_smoke`。
  本机日志：`D:\Window-No-20-Tools\oled-smoke-compile.log`。
- 固件路径：`D:\Window-No-20-Tools\platformio-build\oled_smoke\firmware.bin`；
  SHA-256：`8ca72bf9e82a1285eb8b7c60a7672812f2b710fd31b333018a73a0d93d4c2f4d`。
- 烧录：首次尝试失败，见下述记录。I2C应答与实际显示：未测。三个按钮：未接。

### 首次OLED烧录失败与只读检查

- 四线接入后用户接回USB；CP2102设备状态OK，仍为COM5。确认OLED固件SHA-256与已编译产物一致。
- PlatformIO / esptool 5.4.0在引导程序写入地址0x1000的校验阶段失败：
  `MD5 of file does not match data in flash!`；OLED应用尚未写入，没有进入I2C或显示验收。
- 输入引导文件23520字节，MD5 `bde5bdb324fc2138839012fc9702fb53`；
  首次烧录工具返回的Flash MD5为 `161802a81ba0e86026aabc2ab5ff62bf`。
- 随后仅做只读检查，未重复写Flash：esptool 5.6.0识别ESP32-D0WDQ6 v1.1、Flash 4MB、
  Flash电压绑带值3.3V（不是万用表测量供电稳定性）。
- 从0x1000读回23520字节两次，两份文件完全一致，MD5均为
  `520151e519deb52ecc6aee076d1e2d03`，与输入文件不同。读回镜像解析失败，输入镜像校验和与验证哈希有效。
- 原始证据保存在本机 `D:\Window-No-20-Tools\oled-smoke-upload.log`、
  `oled-bootloader-readback-a.bin`、`oled-bootloader-readback-b.bin`；不提交含设备标识的原始日志。
- 当前未确定底层原因，不能归因于线松、OLED损坏、驱动或Flash芯片故障。
  最后一次读取使用 `--after no-reset`，设备停留在下载模式，须在后续恢复正常烧录与启动。
- 下一步请求用户先断USB，只拔面包板端的灰/白/黑三根，紫色j2及OLED端插头保留；
  再单独接USB，用相同程序文件进行外设隔离对比。先不重做安装、不改产品代码、不整片擦除Flash。
- 排查参考：[乐鑫官方故障排除](https://docs.espressif.com/projects/esptool/en/latest/esp32/troubleshooting.html)。

### 断开三根线后的对比复测

- 用户确认已断USB、拔出面包板端灰/白/黑三根、保留紫色j2，并重新连接USB。
- COM5状态OK；使用同一份固件、同一烧录工具5.4.0及115200波特率重新上传，未改代码、未整片擦除。
  固件SHA-256仍为 `8ca72bf9e82a1285eb8b7c60a7672812f2b710fd31b333018a73a0d93d4c2f4d`。
- 复测成功：引导程序、分区、boot_app0及OLED应用的写入校验全部通过，上传返回0、最终SUCCESS。
  日志位于本机 `D:\Window-No-20-Tools\oled-smoke-upload-isolated.log`。
- 随后按块接收5秒，取得5条 `OLED_ERROR NO_ACK_ON_0x3C_OR_0x3D`。
  这符合电源/数据/时钟三根断开的状态，证明诊断应用已实际运行；不代表屏幕故障或显示验收通过。
  原始记录：本机 `oled-isolated-runtime.log`、`oled-isolated-runtime-result.json`。
- 移除三根并重新上电后成功，支持继续排查外接线路、接触及供电条件，但单次前后对比不能确定具体原因。
- 恢复连线前请求用户用文字确认USB端左右第一根排针的实际行号，核对j2/j19/a17/a14孔位表的前提；不要求照片。

### 灰色电源线单独恢复后的检查

- 用户确认左右第一根排针都在第1行；随后断USB，只接回灰色j19，保留紫色j2，白/黑仍悬空，再接USB。
- COM5设备状态OK。按块采样7.015秒，收到7条 `OLED_ERROR NO_ACK_ON_0x3C_OR_0x3D`，未见欠压或启动/复位标记。
  这符合两根I2C信号线断开的预期，表明本次短时采样内诊断程序仍在运行。
- 本步骤没有重新烧录、没有测量供电电压/电流，不能据此宣称供电在所有负载下都稳定或此前校验问题已完全解决。
- 本机证据：`D:\Window-No-20-Tools\oled-power-only-runtime.log` 和 `oled-power-only-runtime-result.json`。
- 下一步断USB后恢复白色a17、黑色a14，再接USB，读取I2C状态并由用户观察实际画面。

### 恢复两根信号线后：屏幕空白与启动异常

- 用户报告已恢复接线，并确认屏幕完全没有显示；不视为显示验收通过。
- 初次采样输出含非预期字节，控制台GBK编码无法打印替代字符而中断；改为先保存原始字节并转义输出。
- 接着7.016秒块接收取得7条 `OLED_ERROR NO_ACK_ON_0x3C_OR_0x3D`，没有更新日志或复位标记。
  证据：本机 `oled-connected-runtime.bin` / `.log` / `-result.json`。
- 为排除程序只在setup检测、之后保持错误状态的影响，用esptool 5.6.0的HardReset进行一次受控复位，没有重新烧录。
  首轮捕获到ROM启动日志，未见应用启动标记；下一轮捕获到
  `rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)`，未见正常应用日志。
  证据：本机 `oled-connected-reset-runtime.log` 和 `oled-connected-after-reset.log`。
- 当前仍未确定是接触、实际接线、供电还是其他启动因素，不能把未应答直接等同于屏幕损坏。
- 下一步只隔离白色a17时钟线：先断USB、拔面包板端白线，保留紫j2/灰j19/黑a14，再上电采样；不改程序、不重刷、不整套重接。

### 后续排查复核：停止重复拔线流程

- 上述拔线流程后又复现相同现象，用户指出重复操作缺乏推进。现已停止该流程。
- 已重新审查源代码、引脚依据、原始接收数据及下载路径，详细证据和局限见
  [硬件排查复核](hardware-debug-audit.md)。
- 当前板内已切换为纯串口诊断程序；ROM38400压缩写入成功，复位后的引导/分区/应用校验全部通过。
  应用仍未观察到正常 RUNNING，屏幕显示与 I²C 验收均未通过；不能宣布硬件损坏或已修复。
- 部分接收包含不合理的大量重复字节，不能用于统计重启次数；完整复位日志仍提示启动异常。
- 用户没有万用表；下一步优先断电核对当前四线的实际连接，避免重复同一组插拔。

## 2026-11-11：新接法下OLED首次显示验证通过

- 屏幕改装到面包板：SDA d1、SCK d2、VDD d3、GND d4；杜邦线在c1/c2/c3/c4，
  插孔端直接连接取下面包板的ESP32底部长针P21/P22/3V3/GND。SCK线由白改蓝。
- 诊断中曾加载纯串口与独立GPIO回路程序；负对照符合未连通的预期，电阻正对照未做。
  之后按作者新方案恢复原OLED程序；详细过程见[排查复核](hardware-debug-audit.md)。
- 原OLED固件SHA256未变。采用esptool5.6.0、ROM38400压缩烧录，写后哈希通过，返回0。
- 连续8.02秒采样得到7条OLED_UPDATE，时间值从29031到35631递增；
  作者明确反馈“文字、边框和秒数都正常显示”。
- 受控复位后捕获0x3C地址status=0、OLED_INIT 128×64及7条连续更新，未见异常重启。
- 编译：先前已通过、未改源码；本次烧录：通过；I²C应答：通过；用户实物显示观察：通过；
  复位启动：通过。真实芯片身份仍仅确认SSD1306兼容，不把软件配置当作芯片型号证明。
- 当前接线表已更新到固件指南。旧ESP32插在面包板上的孔位表不再使用。
- 此前故障更支持接触/连接条件影响，但安装方式和一根线同时改变，唯一原因未定位。
  没有测供电电压，也没有证明此前屏幕完全无电或元件损坏。
- 三个按钮未接，P0完整交互未验收；产品功能、断电存档、低功耗与续航未测。

## 2026-11-11：第一只按钮诊断就绪

- 用户照片确认四脚按键；用户报告d15/d17/g15/g17，跨中央沟槽。
  接线最终为c15→ESP32另一个空闲GND、h17→P25；用户确认接好并连接USB。
  OLED现有接线保留，未接电阻或电池。
- 新增 `firmware/button_smoke`，显示BUTTON TEST、P25状态、PRESS COUNT和秒数；
  输入逻辑单独位于 `firmware/shared/DebouncedButton.h`，不修改产品行为规则。
- 30ms去抖、按下单次触发、释放事件；启动已按住不伪造计数。启动自检18项覆盖：
  按下/释放抖动、持续按住、再次按下、时间回绕、启动已按住。
- 先添加自检用例，首次构建因输入类尚未实现而失败（缺少头文件，不是行为测试失败）；
  实现后构建通过，无编译警告；真正逻辑用例在烧录后的ESP32上执行通过。
- 平台/板型/库与OLED工程相同：espressif32 8.0.0、esp32dev、Arduino-ESP32 3.3.12、
  SSD1306 2.5.17、GFX 1.13.0、BusIO 1.17.4。
- 编译：通过；程序356656/1310720字节、RAM23692/327680；镜像362704字节，
  SHA256为2d460cd73fb7e270b9fbbcb1d09a4c2542baf3937840042fb9093008a720647f。
- 烧录：esptool5.6.0、ROM38400压缩，只写0x10000应用；55.2秒，哈希通过，返回0。
- 启动采样8.12秒：`DEBOUNCE_SELFTEST PASS checks=18`，0x3C应答，
  `BUTTON_READY ... initial_pressed=0`，随后7条状态raw=1/pressed=0/count=0，
  时间1001～7007ms递增。启动、自检和空闲无误计数通过。
- 随后12秒采样仍为count=0、松手状态；不能将尚未观察到的实际按压写成通过。
- 已请作者只按一下再松开，观察是否变为1；此项与持续按住、再次按下的真实操作尚待反馈。
  屏幕刷新沿用100kHz全帧传输，会暂时阻塞采样，快速连击响应尚未验证。
- 本机日志位于 `D:\Window-No-20-Tools`：button-smoke-red-build.log、button-smoke-build.log、
  button-smoke-rom-upload.log、button-smoke-startup.bin/.log、button-smoke-first-press.bin/.log。
- 编译方法、接线和复现步骤见 `firmware/button_smoke/README.md`。无桌面后台监视器长期运行。

### 第一只按钮实物反馈

- 作者确认按一下计数变1、松开显示RELEASED；随后自行进行多次及快速按压，反馈完全正常。
  作者未提供连击次数/间隔或测试时长，不将该反馈换算为最大响应频率或正式压力测试指标。
- Codex随后6.02秒串口采样取得6条完整状态，均为raw=1、pressed=0、count=18，
  uptime从222380递增到227385，约每1001ms一条；采样期间计数稳定，未见重启。
  初次0.02秒重复数据不用于通过判断，使用后续稳定采样。
- 软件自检、空闲、单次按下/释放与作者提供的连按观察通过。
  2秒持续按住尚无独立量化实物记录；第二、第三只按钮和完整P0尚未验收。
- 证据：本机 `button-one-after-user-press-confirm.bin/.log`，加本次作者的实物文字反馈。
