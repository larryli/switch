# 智能开关网络接口

## 配网

设备在没有 Wifi 网络连接信息配置（即未配网）时会进入配网状态，此时支持微信 AirKiss 和乐鑫 EspTouch 两种自动配网协议。无需设备切换到 AP 状态即可轻松配网。

**注意**：默认不支持微信 AirKiss 的 AES128-CBC 加密 Wifi 密码的配置。如需支持，请额外处理收到的加密密码并解密，然后手工连接网络。

## 局域网发现

一般情况下，设备在连接 Wifi 后都会由 DHCP 分配一个动态局域网 IP 地址。需要解决设备发现（确定其 IP），和设备识别（IP 是不固定的）问题。

设备在连接 Wifi 成功后，会使用 mDNS 协议注册自己以及特定的服务。其中：

- **设备发现**：默认编译会注册到 `_http_switch._tcp.local.` 服务，将会找到设备注册名。同类型的不同设备都会注册到此服务，所以可能会找到多个设备。
- **设备识别**：默认编译会注册 `SWITCH_XXYYZZ.local` 名称，其中 `XXYYZZ` 为 6 位 Mac 地址后 3 位，通过名称可以反查到设备的局域网 IP，同时使用不同的名称识别不同的设备。同类型的不同设备具有不同的 Mac 地址后缀。

## HTTP Restful 接口

设备在连接 Wifi 成功后，会在 80 端口开启 HTTP 服务。

### Restful 通用返回说明

默认返回相应为 `appliction/json`。

- HTTP 状态码：`200` 成功
- HTTP 状态码：`404` 找不到指定的资源
- HTTP 状态码：`422` 请求参数错误
- 返回数据：`success` 整数，`0` 或 `1`，即请求失败或成功
- 返回数据：`message` 字符串，请求失败时的简要错误信息

### `GET /switch` 获取开关状态

- 必需参数：无
- 可选参数：`switch` 整数，为开关编号，从 `1` 开始计数，不超过编译规定的最大开关数目；没有提供此参数时将返回所有开关状态数组
- 返回数据：`success` 整数，`0` 或 `1`，下面的数据只在 `1` 时返回
- 返回数据：`switch` 整数，为开关编号；请求提供 `switch` 参数时返回
- 返回数据：`state` 整数，`0` 或 `1`，即开关已关闭或已打开状态；请求提供 `switch` 参数时返回
- 返回数据：`switches` 数组，包含所有开关的 `switch` 与 `state` 数据；请求未提供 `switch` 参数时返回

### `POST /switch` 修改开关状态

- 必需参数：`state` 整数，`0` 或 `1`，即关闭开关或打开开关
- 可选参数：`switch` 整数，为开关编号，从 `1` 开始计数，不超过编译规定的最大开关数目；没有提供此参数时将修改所有开关
- 可选参数：`switches` 整数；提供此参数将会返回额外的所有开关状态数组
- 返回数据：`success` 整数，`0` 或 `1`，下面的数据只在 `1` 时返回
- 返回数据：`switch` 整数，为开关编号；请求提供 `switch` 参数时返回
- 返回数据：`state` 整数，`0` 或 `1`，即开关已关闭或已打开状态；请求提供 `switch` 参数时返回
- 返回数据：`switches` 数组，包含所有开关的 `switch` 与 `state` 数据；请求未提供 `switch` 参数时或提供 `switches` 参数时返回

## HTTP 直接操作

直接访问设备 IP 地址 80 端口，提供 HTML5 操作界面。