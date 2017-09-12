# 智能开关

红外与 Wifi 遥控开关，最多支持 8 路开关。建议只采用 4 路以下，避免外接额外处理电路。

## 硬件

![面包包接线图](switch_bb.png)

物料清单：

- NodeMCU v3 开发板（板载 D4 Led、D3 Flash 与 Reset 按键）
- 2 通道低电平触发继电器（可选高电平触发，1 路、4 路、双 4 路）
- HX1838 红外接收头模块（需 CarMP3 21 键遥控器，NEC 编码）
- 母对母杜邦线 7 条

注意：

- NodeMCU v3 开发板 ESP6288 12F 板载 Led 接在 D4 GPIO2，低电平触发
- NodeMCU v3 开发板板载 Flash 按键接在 D3 GPIO0，低电平触发
- NodeMCU v3 开发板板载 Reset 按键接在 RST，低电平触发
- 双 4 路继电器依次使用 D1、D2、D6、D7、D0、D8、RX、TX。其中后 4 个 GPIO 均有特殊用途：D0 GPIO16 默认会置高电平，D8 GPIO15 同样，RX 即 D9 GPIO3 用于默认的串口接收数据（写固件用），TX 即 D10 GPIO1 用于默认的串口发送数据（需禁用串口调试输出）。除非清楚相关外围电路的设计，请不要使用后面的 4 个引脚。即**只建议使用一个 4 路继电器**。

## 软件

- `#define SWITCH_COUNT 2` 继电器数量
- `#define SWITCH_TRIG_LOW` 继电器低电平触发，注释掉就是高电平触发
- `#define SWITCH_DEBUG` 开启 TX0 调试输出，注释掉就是关闭调试输出

### 板载按键

- 板载 Reset 按键硬件重启
- 板载 Flash 按键清除配网重启

### 板载 LED

LED 状态 | 说明
:-------:|----
快闪 2 下，亮 0.5 秒 | 正在联网
常亮 | 联网正常
慢闪，亮 0.5 秒，灭 1.5 秒 | 联网错误
持续快闪 | 配网状态
闪 1 下 | 开关动作（只在网络正常时反馈）

### 21 键遥控器（NEC 编码）

- `1`…`8` 键对应 1…8 路继电器切换状态
- `0` 键全关所有继电器
- `100+` 键全开所有继电器
- `EQ` 键对应板载 Flash 键清除配网重启

 按键  | 数值     | 按键  | 数值     | 按键  | 数值
:-----:|---------:|:-----:|---------:|:-----:|---------:
 `CH-` | 0xFFA25D | `CH`  | 0xFF629D | `CH+` | 0xFFE21D
 ⏮   | 0xFF22DD | ⏭   | 0xFF02FD | ⏯   | 0xFFC23D
 `-`   | 0xFFE01F | `+`   | 0xFFA857 | `EQ`  | 0xFF906F
 `0`   | 0xFF6897 | `100+`| 0xFF9867 | `200+`| 0xFFB04F
 `1`   | 0xFF30CF | `2`   | 0xFF18E7 | `3`   | 0xFF7A85
 `4`   | 0xFF10EF | `5`   | 0xFF38C7 | `6`   | 0xFF5AA5
 `7`   | 0xFF42BD | `8`   | 0xFF4AB5 | `9`   | 0xFF52AD

## 参考

- [8 Relay Control With NodeMCU and IR Receiver](http://www.instructables.com/id/8-Relay-Control-With-NodeMCU-and-IR-Receiver-Using/)
- [Using ESP8266 GPIO0/GPIO2/GPIO15](https://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html)
