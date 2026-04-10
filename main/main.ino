#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_ImageReader.h>
#include <SPI.h>
#include <Adafruit_ST77XX.h>


#define SD_CS 4 // SD card select pin
#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8


#define TEXT_SIZE 7

// Custom color for orange (16-bit RGB565)
#define ST77XX_ORANGE 0xFD00  

SdFat SD;// SD card filesystem
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ImageReader reader(SD);         

char low_oil_pressure_warning[] = "LOW OIL\nPRESSURE";
char high_oil_pressure_warning[] = "HIGH OIL\nPRESSURE";
char high_coolant_temp_warning[] = "HIGH\nCOOL\nTEMP";

int oil_pressure = 90;   //update with real variable
int coolant_temp = 200; //update with real variable
int battery_voltage = 12;   //update with real variable


const int BUTTON_PIN = 2; //update with real pin number

// Screen state
enum ScreenState { LOGO, HOME, WARNING };
ScreenState currentScreen = HOME;

// Warning event structure
struct WarningEvent {
  const char* text;
  int* value;
  uint16_t color;
};

WarningEvent warningQueue[5];
int queueCount = 0;

// Timing for warnings (non-blocking)
unsigned long warningStartTime = 0;
const unsigned long WARNING_DURATION = 5000; // ms
bool warningActive = false;

// Forward declarations
void display_text(const char* text, int *int_value, uint16_t color);
void showHomeScreen();
void checkAndEnqueueWarnings();
void processWarnings();
bool isWarningConditionActive(const WarningEvent& w);
bool isWarningAlreadyQueued(const char* text);
bool isCurrentWarning(const char* text);
void showLogo();
void updateHomeScreen();


void setup() {
  Serial.begin(9600);

  tft.init(240, 320);

  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(TEXT_SIZE);
  tft.setTextWrap(false);

  reader.drawBMP("/wsuRLogo.bmp", tft, 0, 0);
  delay(5000);

  currentScreen = HOME;
  showHomeScreen();

}

void loop() {
  // In the future, oil_pressure and coolant_temp will be updated from datalogger

  // 1) Check current sensor values and enqueue any new warnings
  checkAndEnqueueWarnings();

  // 2) Process warning state machine (non-blocking timing, queue, home screen)
  processWarnings();

  //shows logo when button is pressed
  if (!warningActive && digitalRead(BUTTON_PIN) == LOW) {
    currentScreen = LOGO;
    showLogo();
  }

  //uopdates home screen when data changes
  if (!warningActive && currentScreen == HOME) {
    updateHomeScreen();
  }



}

// ---------------------- Warning logic ----------------------

void checkAndEnqueueWarnings() {
  // Low oil pressure (< 100 kPa) -> YELLOW
  if (oil_pressure < 100) {
    if (!isWarningAlreadyQueued(low_oil_pressure_warning) &&
        !isCurrentWarning(low_oil_pressure_warning)) {
      warningQueue[queueCount++] = { low_oil_pressure_warning, &oil_pressure, ST77XX_YELLOW };
    }
  }

  // High oil pressure (> 550 kPa) -> ORANGE
  if (oil_pressure > 550) {
    if (!isWarningAlreadyQueued(high_oil_pressure_warning) &&
        !isCurrentWarning(high_oil_pressure_warning)) {
      warningQueue[queueCount++] = { high_oil_pressure_warning, &oil_pressure, ST77XX_ORANGE };
    }
  }

  // High coolant temp (> 230 F) -> RED
  if (coolant_temp > 230) {
    if (!isWarningAlreadyQueued(high_coolant_temp_warning) &&
        !isCurrentWarning(high_coolant_temp_warning)) {
      warningQueue[queueCount++] = { high_coolant_temp_warning, &coolant_temp, ST77XX_RED };
    }
  }
}

void processWarnings() {
  if (!warningActive) {
    // No active warning
    if (queueCount > 0) {
      // Start showing the first warning in the queue
      WarningEvent &w = warningQueue[0];
      display_text(w.text, w.value, w.color);
      warningStartTime = millis();
      warningActive = true;
      currentScreen = WARNING;
    } else {
      // No warnings queued -> ensure home screen is shown
      if (currentScreen != HOME) {
        showHomeScreen();
        currentScreen = HOME;
      }
    }
  } 
  else {
    // A warning is currently active
    WarningEvent &w = warningQueue[0];

    // Always update displayed value (live update)
    display_text(w.text, w.value, w.color);

    // Has the minimum display time passed?
    if (millis() - warningStartTime >= WARNING_DURATION) {

      //  CASE 1: More than one warning in queue → ALWAYS advance
      if (queueCount > 1) {
        // Remove current warning
        for (int i = 1; i < queueCount; i++) {
          warningQueue[i - 1] = warningQueue[i];
        }
        queueCount--;
        warningActive = false;
        return;  // Next warning will start on next loop
      }

      //  CASE 2: Only one warning → clear ONLY if condition is normal
      if (!isWarningConditionActive(w)) {
        // Remove warning
        for (int i = 1; i < queueCount; i++) {
          warningQueue[i - 1] = warningQueue[i];
        }
        queueCount--;
        warningActive = false;

        // Return to home screen
        showHomeScreen();
        currentScreen = HOME;
      } 
      else {
        // Condition still active → restart timer, keep showing
        warningStartTime = millis();
      }
    }
  }
}


bool isWarningConditionActive(const WarningEvent& w) {
  if (w.text == low_oil_pressure_warning) {
    return (*w.value < 100);
  } else if (w.text == high_oil_pressure_warning) {
    return (*w.value > 550);
  } else if (w.text == high_coolant_temp_warning) {
    return (*w.value > 230);
  }
  return false;
}

bool isWarningAlreadyQueued(const char* text) {
  for (int i = 0; i < queueCount; i++) {
    if (warningQueue[i].text == text) return true;
  }
  return false;
}

bool isCurrentWarning(const char* text) {
  if (!warningActive || queueCount == 0) return false;
  return (warningQueue[0].text == text);
}

// ---------------------- Display helpers ----------------------

void showHomeScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(5);

  tft.println("Battery:");
  tft.println(battery_voltage);

  tft.println("Oil:");
  tft.println(oil_pressure);

  tft.println("Coolant:");
  tft.println(coolant_temp);
}

void updateHomeScreen() {
  static int lastBattery = -9999;
  static int lastOil = -9999;
  static int lastCoolant = -9999;

  // Only redraw if something changed
  if (battery_voltage != lastBattery ||
      oil_pressure != lastOil ||
      coolant_temp != lastCoolant) {

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(5);   // your chosen size

    tft.print("Batt: ");
    tft.println(battery_voltage);

    tft.print("Oil: ");
    tft.println(oil_pressure);

    tft.print("Cool: ");
    tft.println(coolant_temp);

    lastBattery = battery_voltage;
    lastOil = oil_pressure;
    lastCoolant = coolant_temp;
  }
}


void showLogo() {
  tft.fillScreen(ST77XX_BLACK);
  reader.drawBMP("/wsuRLogo.bmp", tft, 0, 0);
}


void display_text(const char* text, int *int_value, uint16_t color) {
  static const char* lastText = "";
  static int lastValue = -999999;
  static uint16_t lastColor = 0;

  // Only redraw if something changed
  if (lastText != text || lastValue != *int_value || lastColor != color) {

    tft.fillScreen(ST77XX_BLACK);

    tft.setCursor(0, 0);
    tft.setTextColor(color);
    tft.setTextSize(TEXT_SIZE);
    tft.setTextWrap(false);

    tft.println(text);

    char str_buf[10];
    sprintf(str_buf, "%d", *int_value);
    tft.println(str_buf);

    lastText = text;
    lastValue = *int_value;
    lastColor = color;
  }
}

