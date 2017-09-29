///
// mDNS，局域网发现与识别
//

#include "switch.h"
#include <ESP8266mDNS.h>

String mdns_name;
static bool _mdns_state; // MDNS 是否已启动

///
// mDNS 配置，使用预定义名称和 mac 地址后三位命名设备
//
void mdns_setup()
{
  String mac = WiFi.macAddress();

  mdns_name = String(SWITCH_NAME) + mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
  mdns_name.toUpperCase();
  _mdns_state = false;
}

///
// 网络连接成功时，开启 mDNS 服务，注册设备名和指定服务
//
void mdns_event(const Event e)
{
  if (e != EVENT_CONNECTED) {
    return;
  }
  if (_mdns_state) {
    MDNS.update();
    return;
  }
  _mdns_state = MDNS.begin(mdns_name.c_str());
  if (!_mdns_state) {
    debug_println(F("[DEBUG] mDNS error"));
    return;
  }
  MDNS.addService(F("http"), F("tcp"), 80);
  MDNS.addService(F(SWITCH_SERV), F("tcp"), 80);
  debug_print(F("[DEBUG] mDNS success: "));
  debug_println(mdns_name);
}
