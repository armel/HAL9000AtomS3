// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Version
#define VERSION "1.2.0"
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
#define MJPEG_BUFFER_SIZE 128 * 128 * 2  // Memory for a single JPEG frame
#define DEST_FS_USES_LITTLEFS

// Dependencies
#include <Preferences.h>
#include <LittleFS.h>
#include <ESP32-targz.h>
#include <M5Unified.h>
#include "MjpegClass.h"
#include "JpegFunc.h"

// Preferences
Preferences preferences;

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

int8_t indice         = 0;
uint8_t limit         = 0;
uint8_t videoCurrent  = 0;
uint8_t videoLast     = 0;
uint8_t brightness    = BRIGHTNESS;
uint8_t brightnessOld = 0;
uint8_t showEye       = 10;
