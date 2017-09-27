///
// HTTP 服务
//

///
// 使用 Flash 保存字符串，节省 RAM 空间
// @see http://espressif.com/sites/default/files/documentation/save_esp8266ex_ram_with_progmem_cn.pdf
//
static const char TYPE_JSON[] PROGMEM = "application/json";
static const char TYPE_HTML[] PROGMEM = "text/html";
static const char TYPE_CSS[] PROGMEM = "text/css";
static const char TYPE_JS[] PROGMEM = "application/javascript";
static const char ERROR_404[] PROGMEM = "{\"success\":0,\"message\":\"Not Found\"}";
static const char ERROR_422[] PROGMEM = "{\"success\":0,\"message\":\"Unprocessable Entity\"}";
static const char HTML_HEAD[] PROGMEM = "<!DOCTYPE HTML>\n<html>\n<head>\n<meta charset='utf-8'>\n\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n<title>智能开关</title>\n\
<link href='/style.css' rel='stylesheet'>\n</head>\n<body>\n<h1>智能开关</h1>\n";
static const char HTML_FOOT[] PROGMEM = "<script src='/turn.js'></script>\n</body>\n</html>";
static const char STYLE_CSS[] PROGMEM = "*{margin:0;padding:0}body{background-color:#f8f8f8}h1{text-align:center;margin:1em}\
h2{margin:.77em 0 .3em 0;padding:0 15px;color:#999;font-size:14px}\
dl{margin:0;background-color:#FFF;line-height:1.47;font-size:17px;overflow:hidden;position:relative}\
dl:before{content:'';position:absolute;left:0;top:0;right:0;height:1px;border-top:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 0;transform:scaleY(0.5);z-index:2}\
dl:after{content:'';position:absolute;left:0;bottom:0;right:0;height:1px;border-bottom:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 100%;transform:scaleY(0.5);z-index:2}\
div{padding:10px 15px;position:relative;display:flex;align-items:center}div:first-child:before{display:none}\
div:before{content:'';position:absolute;left:15px;top:0;right:0;height:1px;border-top:1px solid #e5e5e5;\
color:#e5e5e5;transform-origin:0 0;transform:scaleY(0.5);z-index:2}\
dt{flex:1}dd{font-size:0;text-align:right;color:#999}dd form{display:inline-block}\
dd label{position:relative;display:inline-block;width:60px;height:34px}dd label input{display:none}\
dd span{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;border-radius:34px}\
dd span:before{position:absolute;content:'';height:26px;width:26px;left:4px;bottom:4px;background-color:white;border-radius:50%}\
dd span.on{background-color:#2196F3;box-shadow:0 0 1px #2196F3}dd span.on:before{transform:translateX(26px)}\
form>input{width:90%;border:0;background-color:#1AAD19;position:relative;display:block;margin:0 auto;padding:0 14px;\
box-sizing:border-box;font-size:18px;text-align:center;text-decoration:none;color:#FFF;line-height:2.56;border-radius:5px;overflow:hidden}\
form>input.off{background-color:#E64340}form>input:after{content:'';width:200%;height:200%;position:absolute;top:0;\
left:0;border:1px solid rgba(0,0,0,0.2);transform:scale(0.5);transform-origin:0 0;box-sizing:border-box;border-radius:10px}\
form>input{margin-top:15px}.hidden{display:none}";
static const char TURN_JS[] PROGMEM = "var D=document,F=false,U=undefined,I=i=>D.getElementById(i),\
T=(i,b)=>{I(i).className=b?'':'hidden'},turn=t=>{var f=t.parentNode,d=new FormData();\
Array.from(f.children).forEach(o=>{if(o.name!=U&&o.name!='')d.append(o.name,o.value)});\
var r=new XMLHttpRequest();r.onreadystatechange=()=>{if(r.readyState==4&&r.status==200){ok(r.responseText)}};\
r.open(f.method,f.action);r.send(d);return F},turn2=(t=>turn(t.parentNode)),\
ok=s=>{var o=JSON.parse(s);if(o.success!=U&&o.success==1&&o.switches!=U){\
var on=F,off=F;o.switches.forEach(i=>{var p=I('switch-'+i.switch),s=i.state==1;\
p.querySelector('input[name=state]').value=s?0:1;p.querySelector('span').className=s?'on':'';\
on=on||!s;off=off||s});T('switch-on',on);T('switch-off',off)}},\
M=v=>{D.getElementsByName('switches').forEach(o=>{o.value=v})};\
if(!!window.EventSource){var s=new EventSource('/events');s.onopen=e=>{M(0)};\
s.onerror=e=>{if(e.target.readyState==EventSource.OPEN)M(0);else M(1)};s.onmessage=e=>{ok(e.data)}}";

static AsyncWebServer server(80);
AsyncEventSource events("/events");

///
// HTTP 服务配置
//
void server_setup()
{
  server.on("/", server_root);
  server.on("/style.css", [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get style.css"));
    request->send(200, FPSTR(TYPE_CSS), FPSTR(STYLE_CSS));
  });
  server.on("/turn.js", [](AsyncWebServerRequest *request) {
    debug_println(F("[DEBUG] Server receive get turn.js"));
    request->send(200, FPSTR(TYPE_JS), FPSTR(TURN_JS));
  });
  server.on("/switch", HTTP_GET, server_get_switch);
  server.on("/switch", HTTP_POST, server_post_switch);
  events.onConnect([](AsyncEventSourceClient *client){
    String content = String(F("{\"success\":1"));
    content += server_get_switches();
    content += F("}");
    client->send(content.c_str(), NULL, millis(), 1000);
  });
  server.addHandler(&events);
  server.begin();
  debug_println(F("[DEBUG] Server start"));
}

void server_update()
{
  String content = String(F("{\"success\":1"));
  content += server_get_switches();
  content += F("}");
  events.send(content.c_str(), NULL, millis(), 1000);
}

static void server_root(AsyncWebServerRequest *request)
{
  bool can_on, can_off;
  String content = String(FPSTR(HTML_HEAD));

  debug_println(F("[DEBUG] Server receive get home page"));
  can_on = can_off = false;
  content += F("<h2>开关列表</h2>\n<dl>\n");
  for (int i = 0; i < SWITCH_COUNT; i++) {
    bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
    String name = String(i + 1);
    content += F("<div>\n<dt>开关 #");
    content += name;
    content += F("</dt>\n<dd>");
    content += server_form1(name, !state, state ? F("关闭") : F("打开"));
    content += F("</dd>\n</div>\n");
    can_on = can_on || !state;
    can_off = can_off || state;
  }

  content += F("</dl>\n<h2>批量开关</h2>\n");
  content += server_form2(can_on, true, F("全部打开"));
  content += server_form2(can_off, false, F("全部关闭"));
  content += FPSTR(HTML_FOOT);
  request->send(200, FPSTR(TYPE_HTML), content);
}

static void server_get_switch(AsyncWebServerRequest *request)
{
  String content = String(F("{\"success\":1"));

  if (request->hasArg("switch")) {
    String name = request->arg("switch");
    int i = name.toInt() - 1;
    if (i < 0 || i >= SWITCH_COUNT) {
      request->send(404, FPSTR(TYPE_JSON), FPSTR(ERROR_404));
      return;
    }
    debug_print(F("[DEBUG] Server receive get switch #"));
    debug_println(name);
    content += F(",\"switch\":");
    content += name;
    content += F(",\"state\":");
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? F("1") : F("0");
    content += F("}");
    debug_print(F("[DEBUG] Server send: "));
    debug_println(content);
    request->send(200, FPSTR(TYPE_JSON), content);
    return;
  }
  debug_println(F("[DEBUG] Server receive get switches"));
  content += server_get_switches();
  content += F("}");
  debug_print(F("[DEBUG] Server send: "));
  debug_println(content);
  request->send(200, FPSTR(TYPE_JSON), content);
}

static void server_post_switch(AsyncWebServerRequest *request)
{
  String content = String(F("{\"success\":1"));
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
        request->send(404, FPSTR(TYPE_JSON), FPSTR(ERROR_404));
        return;
      }
      debug_print(F("[DEBUG] Server receive post switch #"));
      debug_print(name);
      debug_print(F(" to "));
      debug_println(state);
      led_switch();
      switch_turn(i, b);
      oled_refresh();
      if (ret_type > 0) {
        content += F(",\"switch\":");
        content += name;
        content += F(",\"state\":");
        content += state;
        if (ret_type > 1) {
          content += server_get_switches();
        }
      }
      content += F("}");
      debug_print(F("[DEBUG] Server send: "));
      debug_println(content);
      request->send(200, FPSTR(TYPE_JSON), content);
      server_update();
      return;
    }

    debug_print(F("[DEBUG] Server receive post switches to "));
    debug_println(state);
    led_switch();
    for (int i = 0; i < SWITCH_COUNT; i++) {
      switch_turn(i, b);
    }
    oled_refresh();
    if (ret_type > 0) {
      content += server_get_switches();
    }
    content += F("}");
    debug_print(F("[DEBUG] Server send: "));
    debug_println(content);
    request->send(200, FPSTR(TYPE_JSON), content);
    server_update();
    return;
  }
  request->send(422, FPSTR(TYPE_JSON), FPSTR(ERROR_422));
}

static String server_get_switches()
{
  String content = String(F(",\"switches\":["));
  for (int i = 0; i < SWITCH_COUNT; i++) {
    content += F("{\"switch\":");
    content += String(i + 1);
    content += F(",\"state\":");
    content += (digitalRead(SWITCHES[i]) == SWITCH_ON) ? F("1") : F("0");
    content += F("}");
    if (i < SWITCH_COUNT - 1) {
      content += F(",");
    }
  }
  content += F("]");
  return content;
}

static String server_form1(String i, bool state, String name)
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
  content += F("' onclick='return turn2(this)'><span");
  if (!state) {
    content += F(" class='on'");
  }
  content += F("></span></label>\n</form>\n");
  return content;
}

static String server_form2(bool show, bool state, String name)
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
