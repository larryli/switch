#include "switch.h"
#include <U8g2lib.h> // @see https://github.com/olikraus/u8g2
#include "oled_font.h"

static U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, D7, D6);
static int start = 0;
static int selected = 0;
static int count;
static String top_menu[11];

void oled_setup()
{
  u8g2.begin();
  u8g2.setFont(oled_font);
  count = SWITCH_COUNT + 1;
}

void oled_loop()
{
  u8g2.firstPage();
  do {
    int n = -start;
    bool can_on, can_off;
    oled_str(false, n++, String("智能开关"), false, false);
    can_on = can_off = false;
    for (int i = 0; i < SWITCH_COUNT; i++) {
      bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
      oled_str(selected == n + start, n++, String("开关 #") + String(i + 1), true, state);
      can_on = can_on || !state;
      can_off = can_off || state;
    }
    count = SWITCH_COUNT + 1;
    if (can_on) {
      oled_str(selected == n + start, n++, "全部打开", false, false);
      count++;
    }
    if (can_off) {
      oled_str(selected == n + start, n++, "全部关闭", false, false);
      count++;
    }
    
    if (count > 4 && start > count - 5) {
      start = count - 5;
      selected = count - 1;
    } else if (selected >= count) {
      selected = count - 1;
    }
  } while (u8g2.nextPage());
}

void oled_up()
{
  selected--;
  if (selected < start) {
    start--;
    if (start < 0) {
      selected = start = 0;
    }
  }
}

void oled_down()
{
  selected++;
  if (count > 4 && selected > start + 4) {
    start++;
    if (start > count - 5) {
      start = count - 5;
      selected = count - 1;
    }
  } else if (selected >= count) {
    selected = count - 1;
  }
}

void oled_select()
{
  if (selected > 0 && selected < SWITCH_COUNT + 1) {
    int i = selected - 1;
    led_switch();
    switch_toggle(i);
  } else {
    bool can_on = false;
    if (selected < SWITCH_COUNT + 2) {
      for (int i = 0; i < SWITCH_COUNT; i++) {
        bool state = (digitalRead(SWITCHES[i]) == SWITCH_ON);
        can_on = can_on || !state;
      }
    }
    led_switch();
    for (int i = 0; i < SWITCH_COUNT; i++) {
      switch_turn(i, can_on);
    }
  }
}

static void oled_str(bool selected, int pos, String str, bool extra, bool state)
{
  int y;
  
  if (pos < 0 || pos > 4) {
    return;
  }
  y = pos * 13 + 11;
  u8g2.setDrawColor(1);
  if (selected) {
    u8g2.drawBox(0, y - 11, 128, 13);
    u8g2.setDrawColor(0);
  }
  u8g2.drawUTF8(1, y, str.c_str());
  if (extra) {
    u8g2.drawUTF8(115, y, state ? "●" : "○");
  }
}

