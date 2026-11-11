#pragma once
#include <Adafruit_SSD1306.h>
#include "RoomModel.h"
void drawRoomView(Adafruit_SSD1306 &d, const RoomModel &m, uint16_t minuteOfDay);
