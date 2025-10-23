#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

#define USE_SD_CARD

#define SD_CS    4 // SD card select pin
#define TFT_CS  10 // TFT select pin
#define TFT_DC   8 // TFT display/command pin
#define TFT_RST  9 // Or set to -1 and connect to Arduino RESET pin

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {

  Serial.begin(9600);

  tft.init(240, 320);           // Init ST7789 320x240

  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
      Serial.println(F("SD begin() failed"));
      for(;;); // Fatal error, do not continue
    }

  tft.fillScreen(ST77XX_BLACK);

  Serial.print(F("Loading wsuRLogo2.bmp to screen..."));
  tft.setRotation(1);
  reader.drawBMP("/wsuRL2.bmp", tft, 0, 0);

  delay(5000);

  Serial.print(F("Loading alculver.bmp to screen..."));
  tft.setRotation(1);
  reader.drawBMP("/alculver.bmp", tft, 0, 0);

  delay(8000);

  Serial.print(F("Loading wsuRLogo2.bmp to screen..."));
  tft.setRotation(1);
  reader.drawBMP("/wsuRL2.bmp", tft, 0, 0);

}

void loop() {}
