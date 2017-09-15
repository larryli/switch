#include "switch.h"

void reset_setup()
{
  pinMode(RESET_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(RESET_BTN), reset_handle, FALLING);
}

static void reset_handle()
{
  DPRINTLN("[DEBUG] Push reset button");
  switch_reset();
}

