///
// HTTP 服务
//
// 升级固件
// curl -F "image=@switch.bin" http://SWITCH_XXYYzZ.local/update

#include "switch.h"
<<<<<<< HEAD
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
=======
>>>>>>> master

///
// 使用 Flash 保存字符串，节省 RAM 空间
// @see http://espressif.com/sites/default/files/documentation/save_esp8266ex_ram_with_progmem_cn.pdf
//
static const char _server_error_404[] PROGMEM = "{\"success\":0,\"message\":\"Not Found\"}";
static const char _server_error_422[] PROGMEM = "{\"success\":0,\"message\":\"Unprocessable Entity\"}";
static const char _server_html_head[] PROGMEM = "<!DOCTYPE HTML>\n<html>\n<head>\n<meta charset='utf-8'>\n\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n<title>智能开关</title>\n\
<link rel='icon' href='/favicon.ico' type='image/x-icon'>\n\
<link href='/style.css' rel='stylesheet'>\n</head>\n<body>\n<h1>智能开关</h1>\n";
static const char _server_html_foot[] PROGMEM = "<script src='/turn.js'></script>\n</body>\n</html>";

<<<<<<< HEAD
static ESP8266WebServer server(80);
#ifdef SWITCH_DEBUG
static ESP8266HTTPUpdateServer update(true);
#else
static ESP8266HTTPUpdateServer update;
#endif
=======
// CSS & JS gzip data
#include "server_data.h"

static AsyncWebServer _server(80);
static AsyncEventSource _events("/events");
>>>>>>> master

///
// HTTP 服务配置
//
void server_setup()
{
<<<<<<< HEAD
  server.on("/", server_root);
  server.on("/style.css", server_style_css);
  server.on("/turn.js", server_turn_js);
  server.on("/switch", HTTP_GET, server_get_switch);
  server.on("/switch", HTTP_POST, server_post_switch);
  update.setup(&server);
}
=======
  _server.on("/", HTTP_GET, _server_root);
  _server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get favicon.ico"));
    _server_static(request, "image/x-icon", _server_favicon_data, SERVER_FAVICON_LEN, SERVER_FAVICON_ETAG);
  });
  _server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get style.css"));
    _server_static(request, "text/css", _server_style_data, SERVER_STYLE_LEN, SERVER_STYLE_ETAG);
  });
  _server.on("/turn.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get turn.js"));
    _server_static(request, "application/javascript", _server_turn_data, SERVER_TURN_LEN, SERVER_TURN_ETAG);
  });
  _server.on("/switch", HTTP_GET, _server_get_switch);
  _server.on("/switch", HTTP_POST, _server_post_switch);
  _events.onConnect([](AsyncEventSourceClient *client){
    String content = String("{\"success\":1");
>>>>>>> master

    content += _server_get_switches();
    content += "}";
    client->send(content.c_str(), NULL, millis());
  });
  _server.addHandler(&_events);
  _server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });
  _server.begin();
  debug_println(F("[DEBUG] Server start"));
}

void server_event(const Event e)
{
  if (e != EVENT_REFRESH) {
    return;
  }
  String content = String("{\"success\":1");

  content += _server_get_switches();
  content += "}";
  _events.send(content.c_str(), NULL, millis());
}

static void _server_root(AsyncWebServerRequest *request)
{
  bool can_on, can_off;
  String content = String(FPSTR(_server_html_head));

  debug_println(F("[DEBUG] Server receive get home page"));
  can_on = can_off = false;
  content += F("<h2>开关列表</h2>\n<dl>\n");
  for (int i = 0; i < SWITCH_COUNT; i++) {
    bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
    String name = String(i + 1);
    content += F("<div>\n<dt>开关 #");
    content += name;
    content += F("</dt>\n<dd>");
    content += _server_form1(name, !state, state ? F("关闭") : F("打开"));
    content += F("</dd>\n</div>\n");
    can_on = can_on || !state;
    can_off = can_off || state;
  }

  content += F("</dl>\n<h2>批量开关</h2>\n");
  content += _server_form2(can_on, true, F("全部打开"));
  content += _server_form2(can_off, false, F("全部关闭"));
  content += FPSTR(_server_html_foot);
  request->send(200, F("text/html"), content);
}

static void _server_get_switch(AsyncWebServerRequest *request)
{
  String content = String("{\"success\":1");

  if (request->hasArg("switch")) {
    String name = request->arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      request->send(404, "application/json", FPSTR(_server_error_404));
      return;
    }
    debug_print(F("[DEBUG] Server receive get switch #"));
    debug_println(name);
    content += ",\"switch\":";
    content += name;
    content += ",\"state\":";
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    debug_print(F("[DEBUG] Server send: "));
    debug_println(content);
    request->send(200, "application/json", content);
    return;
  }
  debug_println(F("[DEBUG] Server receive get switches"));
  content += _server_get_switches();
  content += "}";
  debug_print(F("[DEBUG] Server send: "));
  debug_println(content);
  request->send(200, "application/json", content);
}

static void _server_post_switch(AsyncWebServerRequest *request)
{
  String content = String("{\"success\":1");
  int ret_type = 1;

  if (request->hasArg("switches")) {
    String s = request->arg("switches");
    ret_type = (s == "0") ? 0 : 2;
  }
  if (request->hasArg("state")) {
    String state = request->arg("state");
    bool b = (state == "0") ? false : true;

    if (request->hasArg("switch")) {
      String name = request->arg("switch");
      int i = name.toInt() - 1;

      if (i < 0 || i >= SWITCH_COUNT) {
        request->send(404, "application/json", FPSTR(_server_error_404));
        return;
      }
      debug_print("[DEBUG] Server receive post switch #");
      debug_print(name);
      debug_print(" to ");
      debug_println(state);
      switch_event(Event(i + (b ? EVENT_1_ON : EVENT_1_OFF)));
      if (ret_type > 0) {
        content += ",\"switch\":";
        content += name;
        content += ",\"state\":";
        content += state;
        if (ret_type > 1) {
          content += _server_get_switches();
        }
      }
      content += "}";
      debug_print("[DEBUG] Server send: ");
      debug_println(content);
      request->send(200, "application/json", content);
      return;
    }

    debug_print("[DEBUG] Server receive post switches to ");
    debug_println(state);
    switch_event(b ? EVENT_ON : EVENT_OFF);
    if (ret_type > 0) {
      content += _server_get_switches();
    }
    content += "}";
    debug_print("[DEBUG] Server send: ");
    debug_println(content);
    request->send(200, "application/json", content);
    return;
  }
  request->send(422, "application/json", FPSTR(_server_error_422));
}

static String _server_get_switches()
{
  String content = String(",\"switches\":[");
  for (int i = 0; i < SWITCH_COUNT; i++) {
    content += "{\"switch\":";
    content += String(i + 1);
    content += ",\"state\":";
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? "1" : "0";
    content += "}";
    if (i < SWITCH_COUNT - 1) {
      content += ",";
    }
  }
  content += "]";
  return content;
}

static String _server_form1(String i, bool state, String name)
{
  String content = String(F("<form action='/switch' method='post' id='switch-"));
  content += i;
  content += F("'>\n<input type='hidden' name='switches' value='1'>\n<input type='hidden' name='switch' value='");
  content += i;
  content += F("'>\n<input type='hidden' name='state' value='");
  content += state ? F("1") : F("0");
  content += F("'>\n");
  content += F("<label><input type='submit' value='");
  content += name;
  content += F("' onclick='return turn(this,1)'><span");
  if (!state) {
    content += F(" class='on'");
  }
  content += F("></span></label>\n</form>\n");
  return content;
}

static String _server_form2(bool show, bool state, String name)
{
  String content = String(F("<form action='/switch' method='post' id='switch-"));
  content += state ? F("on") : F("off");
  content += F("'");
  if (!show) {
    content += F(" class='hidden'");
  }
  content += F(">\n<input type='hidden' name='switches' value='1'><input type='hidden' name='state' value='");
  content += state ? F("1") : F("0");
  content += F("'>\n<input type='submit' value='");
  content += name;
  content += F("'");
  if (!state) {
    content += F(" class='off'");
  }
  content += F(" onclick='return turn(this)'>\n</form>\n");
  return content;
}

static void _server_static(AsyncWebServerRequest *request, const String& type, const uint8_t * data, size_t len, const String& etag)
{
  if (request->header("If-Modified-Since") == _server_last) {
    debug_println(F("[DEBUG] Not modified"));
    request->send(304); // Not modified
  } else if (request->hasHeader("If-None-Match") && request->header("If-None-Match").equals(etag)) {
    AsyncWebServerResponse * response = new AsyncBasicResponse(304); // Not modified

    debug_println(F("[DEBUG] ETAG Not modified"));
    response->addHeader("Cache-Control", "max-age=3600");
    response->addHeader("ETag", etag);
    request->send(response);
  } else {
    AsyncWebServerResponse *response = request->beginResponse_P(200, type, data, len);

    debug_println(F("[DEBUG] GZIP output"));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Last-Modified", _server_last);
    response->addHeader("Cache-Control", "max-age=3600");
    response->addHeader("ETag", etag);
    request->send(response);
  }
}
