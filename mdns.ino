#include "switch.h"
#include <ESP8266mDNS.h>

static String mdns_name;

void mdns_setup()
{
  String mac = WiFi.macAddress();
  mdns_name = String(SWITCH_NAME) + mac.substring(9, 11) + mac.substring(12, 14) + mac.substring(15, 17);
  mdns_name.toUpperCase();
}

bool mdns_start()
{
  if (!MDNS.begin(mdns_name.c_str())) {
    debug_println(F("[DEBUG] mDNS error"));
    return false;
  }
  MDNS.addService(F("http"), F("tcp"), 80);
  MDNS.addService(SWITCH_SERV, F("tcp"), 80);
  debug_print(F("[DEBUG] mDNS success: "));
  debug_println(mdns_name);
  return true;
}

