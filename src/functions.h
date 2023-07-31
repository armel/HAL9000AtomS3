// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw) {
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  M5.Displays(0).pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

// Pixel drawing callback
static int mjpegDrawCallback(JPEGDRAW *pDraw) {
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long start = millis();
  M5.Displays(0).pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  total_show_video += millis() - start;
  return 1;
}

// List files on LittleFS
void getVideoList(File dir) {
  String tmp;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    if (strstr(entry.name(), "/.") == NULL && strstr(entry.name(), ".mjpg.gz") != NULL) {
      M5.Displays(0).setTextPadding(128);
      M5.Displays(0).setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Displays(0).setTextDatum(CC_DATUM);

      tmp = entry.name();

      M5.Displays(0).drawString(tmp, 64, 90);

      videoFilename[limit] = entry.name();
      limit++;
      delay(50);
    }

    if (entry.isDirectory() && strstr(entry.name(), "/.") == NULL) {
      Serial.println(entry.name());
      getVideoList(entry);
    }

    entry.close();
  }
}

// Check button
void checkButton(void *pvParameters) {
  uint8_t btnA;
  int8_t step = 2;

  for (;;) {
    skip = false;

    M5.update();

    btnA = M5.BtnA.isPressed();

    if (btnA) {
      brightnessOld += step;
      if(step > 0 && brightnessOld >= 254)
      {
        step = -step;
      }
      else if(step < 0 && brightnessOld <= 2)
      {
        step = -step;
      }
      
      if (brightnessOld != brightness)
      {
        brightness = brightnessOld;
        M5.Displays(0).setBrightness(brightness);
        preferences.putUInt("brightness", brightness);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Progress callback
void myProgressCallback(uint8_t progress) {
  static int8_t myLastProgress = -1;
  if (myLastProgress != progress) {
    if (myLastProgress == -1) {
      Serial.print("Progress: ");
    }
    myLastProgress = progress;
    switch (progress) {
      case 0:
        Serial.print("0% ▓");
        break;
      case 25:
        Serial.print(" 25% ");
        break;
      case 50:
        Serial.print(" 50% ");
        break;
      case 75:
        Serial.print(" 75% ");
        break;
      case 100:
        Serial.print("▓ 100%\n");
        myLastProgress = -1;
        break;
      default:
        if (progress < 100) Serial.print("▓");
        break;
    }
    M5.Displays(0).fillRect(14, 85, progress, 4, TFT_WHITE);
  }
}

// Boot loader
boolean boot() {
  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: File System Mount Failed!"));
  } else {
    jpegDraw(JPEG_LOGO, jpegDrawCallback, true /* useBigEndian */, 0 /* x */, 0 /* y */, 128 /* widthLimit */,
             128 /* heightLimit */);

    // Get video files
    root = LittleFS.open("/");

    M5.Displays(0).setFont(0);
    M5.Displays(0).setTextDatum(CC_DATUM);
    M5.Displays(0).setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Displays(0).drawString("HAL9000", 64, 40);
    M5.Displays(0).drawString("Version " + String(VERSION), 64, 50);
    M5.Displays(0).drawString(" by F4HWN", 64, 60);
    M5.Displays(0).drawString("Loading kernel", 64, 80);
    getVideoList(root);

    for (uint8_t i = 0; i < 5; i++) {
      M5.Displays(0).drawString(" ", 64, 110);
      delay(250);
      M5.Displays(0).drawString("Loading complete", 64, 110);
      delay(250);
    }
  }
  return true;
}

// Eye loader
boolean eye() {
  M5.Displays(0).clear();
  M5.Displays(0).setBrightness(0);

  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: File System Mount Failed!"));
  } else {
    jpegDraw(JPEG_EYE, jpegDrawCallback, true /* useBigEndian */, 0 /* x */, 0 /* y */, 128 /* widthLimit */,
             128 /* heightLimit */);
  }

  for (uint8_t i = 0; i <= brightness; i++) {
    M5.Displays(0).setBrightness(i);
    delay(50);
  }

  delay(1000);

  for (uint8_t i = brightness; i >= 1; i--) {
    M5.Displays(0).setBrightness(i);
    delay(50);
  }

  return true;
}

// Video init
void videoInit() {
  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: File System Mount Failed!"));
  } else {
    M5.Displays(0).setBrightness(brightness);
  }
}

// Video
void video() {
  uint8_t counter = 0;
  String cover;

  total_frames       = 0;
  total_read_video   = 0;
  total_decode_video = 0;
  total_show_video   = 0;

  // Gunzip
  tarGzFS.begin();

  while (1) {
    if (RANDOM == 1) {
      while (videoCurrent == videoLast) {
        videoCurrent = random(limit);  // Returns a pseudo-random integer between 0 and number of video files
      }
    } else {
      indice       = (indice++ < (limit - 1)) ? indice : 0;
      videoCurrent = indice;
    }

    // Serial.printf("%d %s\n", videoCurrent, videoFilename[videoCurrent]);

    cover = videoFilename[videoCurrent];
    cover.replace(".mjpg.gz", ".jpg");

    // M5.Displays(0).drawJpgFile(LittleFS, "/" + cover, 0, 0);

    jpegDraw(("/" + cover).c_str(), jpegDrawCallback, true /* useBigEndian */, 0 /* x */, 0 /* y */,
             128 /* widthLimit */, 128 /* heightLimit */);

    load = true;

    GzUnpacker *GZUnpacker = new GzUnpacker();
    GZUnpacker->haltOnError(true);                   // stop on fail (manual restart/reset required)
    GZUnpacker->setupFSCallbacks(targzTotalBytesFn,
                                 targzFreeBytesFn);  // prevent the partition from exploding, recommended
    // GZUnpacker->setGzProgressCallback( BaseUnpacker::defaultProgressCallback ); // targzNullProgressCallback or
    // defaultProgressCallback
    GZUnpacker->setGzProgressCallback(myProgressCallback);  // targzNullProgressCallback or defaultProgressCallback
    GZUnpacker->setLoggerCallback(BaseUnpacker::targzPrintLoggerCallback);  // gz log verbosity

    if (!GZUnpacker->gzExpander(tarGzFS, ("/" + videoFilename[videoCurrent]).c_str(), tarGzFS, "/tmp.mjpg")) {
      Serial.printf("gzExpander failed with return code #%d", GZUnpacker->tarGzGetError());
    }

    mjpegFile         = LittleFS.open("/tmp.mjpg", "r");
    uint8_t *mjpegBuf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);

    if (!mjpegFile || mjpegFile.isDirectory()) {
      Serial.print("ERROR: Failed to open ");
      Serial.print(videoFilename[videoCurrent]);
      Serial.println(" file for reading");
    } else {
      if (!mjpegBuf) {
        Serial.println(F("mjpegBuf malloc failed!"));
      } else {
        Serial.println(F("MJPEG start"));

        start_ms = millis();
        curr_ms  = millis();
        mjpegClass.setup(&mjpegFile, mjpegBuf, mjpegDrawCallback, true, 0, 0, 128, 128);
        while (mjpegFile.available()) {
          // Skip
          if (skip) {
            Serial.println(F("MJPEG skip"));
            mjpegFile.close();
            LittleFS.remove("/tmp.mjpg");
            free(mjpegBuf);
            videoLast = videoCurrent;
            counter++;
            return;
          }
          // Read video
          mjpegClass.readMjpegBuf();
          total_read_video += millis() - curr_ms;
          curr_ms = millis();

          // Play video
          mjpegClass.drawJpg();

          if (load == true) {
            load = false;
          }

          total_decode_video += millis() - curr_ms;
          curr_ms = millis();
          total_frames++;
          delay(25);  // Hack for AtomS3 only !!!
        }
        Serial.println(F("MJPEG end"));
        mjpegFile.close();
        LittleFS.remove("/tmp.mjpg");
        free(mjpegBuf);
      }
    }

    videoLast = videoCurrent;

    counter++;

    Serial.printf("%d %d \n", counter, limit);
    if (counter >= showEye) {
      eye();
      videoInit();
      counter = 0;
    }
  }
}