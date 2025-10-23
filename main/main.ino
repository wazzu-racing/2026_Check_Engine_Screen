#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <SPI.h>

#include <stdlib.h>
#include <LibPrintf.h>

#include <time.h>

#define USE_SD_CARD

#define SD_CS    4 // SD card select pin
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

#define TEXT_SIZE 7

#define WARNINGS_ARRAY_SIZE 20

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

char warnings_array[WARNINGS_ARRAY_SIZE][50] = {{0}};

char low_oil_pressure_warning[] = "LOW OIL\nPRESSURE";
char high_oil_pressure_warning[] = "HIGH OIL\nPRESSURE";
char low_coolant_temp_warning[] = "LOW\nCOOL\nTEMP";
char high_coolant_temp_warning[] = "HIGH\nCOOL\nTEMP";

char displayed_text[100] = "";

int oil_pressure = 850;
int coolant_temp = 2500;

double first_time = time(NULL);

void setup() {
  Serial.begin(9600);

  tft.init(240, 320);

  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }

  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextWrap(false);

  reader.drawBMP("/wsuRL2.bmp", tft, 0, 0);

  delay(5000);
}



void loop() {
  //Oil pressure checks
  if(oil_pressure/10  < 10) {
    display_text(low_oil_pressure_warning, &oil_pressure, 10);
    delay(5000);
  }
  if(oil_pressure/10  > 80) {
    display_text(high_oil_pressure_warning, &oil_pressure, 10);
    delay(5000);
  }
  // Coolant temp checks
  if(coolant_temp/10  < 195) {
    display_text(low_coolant_temp_warning, &coolant_temp, 10);
    delay(5000);
  }
  if(coolant_temp/10  > 220) {
    display_text(high_coolant_temp_warning, &coolant_temp, 10);
    delay(5000);
  }
}

void display_text(char* text, int * int_value, int divisor) {
  int16_t text_x1, text_y1;
  uint16_t text_length, text_height;

  printf("displayed text at check: %c and text at check: %c", *displayed_text, *text);

  // Make sure that the loop function does not call display text when the text is animating in or is already on screen.
  if (displayed_text == text) return;

  // Copy the text argument into the displayed_text var.
  strcpy(displayed_text, text);

  delay(500);
  tft.fillScreen(ST77XX_BLACK); // Fill screen with black to get rid of all other text.
  delay(500);

  // Print the text onto the screen in white
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(F(text));

  char str_buf[5];
  sprintf(str_buf, "%.1lf", (double)(*int_value)/divisor);
  tft.println(F(str_buf));

  // Get the bounds of the text that was displayed above.
  tft.getTextBounds(text, 0, 0, &text_x1, &text_y1, &text_length, &text_height);

  delay(500);
  tft.fillRect(text_x1, text_y1, text_length, text_height, ST77XX_BLACK); // Fill in black where the text is using the 
  delay(500);

  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(F(text));

  delay(500);
  tft.fillRect(text_x1, text_y1, text_length, text_height, ST77XX_BLACK);
  delay(500);

  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(F(text));
}