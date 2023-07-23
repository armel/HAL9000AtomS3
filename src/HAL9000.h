// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Version
#define VERSION "1.1.0"
#define AUTHOR  "F4HWN"
#define NAME    "HAL9000"

#define RANDOM 1  // Set to 1 for random view
#define CORE   1
#define CORE2  2
#define CORES3 3
#define ATOMS3 4

// Others define
#define JPEG_LOGO         "/HAL9000.jpg"
#define JPEG_EYE          "/HAL9000-eye.jpg"
#define MJPEG_BUFFER_SIZE 32768 * 2  // Memory for a single JPEG frame
#define DEST_FS_USES_LITTLEFS

// Dependencies
#include <LittleFS.h>
#include <ESP32-targz.h>
#include <M5Unified.h>
#include <Arduino_GFX_Library.h>
#include "MjpegClass.h"
#include "JpegFunc.h"

// Variables
static MjpegClass mjpegClass;

static int total_frames                 = 0;
static unsigned long total_read_video   = 0;
static unsigned long total_decode_video = 0;
static unsigned long total_show_video   = 0;
static unsigned long start_ms, curr_ms;

fs::File root;
fs::File mjpegFile;

String videoFilenameMedium[128];
String videoFilenameSmall[128];

boolean load = false;
boolean skip = false;

int8_t indice        = 0;
uint8_t limit        = 0;
uint8_t videoCurrent = 0;
uint8_t videoLast    = 0;
uint8_t brightness   = 32;
uint8_t showEye      = 10;

#if BOARD == ATOMS3
// #define GFX_DEV_DEVICE ARDUINO_M5Stack_ATOMS3
#define GFX_BL 16
Arduino_DataBus *bus =
  new Arduino_ESP32SPI(33 /* DC */, 15 /* CS */, 17 /* SCK */, 21 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_GC9107(bus, 34 /* RST */, 0 /* rotation */, true /* IPS */);
#endif
