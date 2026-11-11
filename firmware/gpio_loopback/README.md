# GPIO 与信号线回路诊断

仅用于当前启动异常排查，不是产品固件。不使用 OLED 库、I²C、联网或存档。

## 验证条件与顺序

1. OLED 的四根线在屏幕端全部拔下，面包板端白 a17、黑 a14 保留。
   未接电阻时发送 `T`，预期 FAIL：两脚各自的弱上拉为高，拉低一脚时另一脚不应跟随。
   这项负对照若意外 PASS，先查两路是否已经短接，不进入正对照。
2. 断 USB，将同一只 1kΩ 电阻两端分别插进白、黑两根线空出的母头。
   紫/灰线保持悬空，OLED 完全不接，电阻引脚不碰其他金属。然后再接 USB。
3. Codex 发送 `T`。四项应全部成功：P21拉低/释放与P22拉低/释放时，两脚读值均应跟随为0/1。
   结果应为 `LOOPBACK_RESULT passed=4 total=4 result=PASS`。
4. 完成后断 USB，再拆回路。不得带着回路电阻接回屏幕。

代码采用两路开漏输出及内部弱上拉，只主动拉低，不输出强高电平；1kΩ限制意外连接电流。
参考[乐鑫 GPIO 模式说明](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32/api-reference/peripherals/gpio.html)。
通过只能支持当前两脚、两根线及面包板接点能传递低速数字电平；不代表OLED、供电或I²C时序已通过。

## 构建与实际状态

- 板型 esp32dev，平台 espressif32 8.0.0，Arduino-ESP32 3.3.12，无额外库。
- 构建：`python -m platformio run --project-dir firmware/gpio_loopback --environment gpio_loopback`。
- 本机使用 `firmware/README.md` 中的专用 Python 和工具目录；独立构建目录
  `D:\Window-No-20-Tools\platformio-build-gpio`，不覆盖之前产物。
- 上传采用已验证的 esptool 5.6.0、ROM、38400、压缩路径，只写现有应用分区0x10000。
  不直接采用默认stub上传来复现已有校验异常。
- 编译、上传、负对照、正对照的实测状态分别记入 `docs/hardware-debug-audit.md`，不能由编译推定硬件通过。
