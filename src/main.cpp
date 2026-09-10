#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Function Declarations ----------

void showWelcomeScreen();
void checkCountryButtons();
void checkModeButton();
void handleCountry(int country);
void showFacts(int country);
void showStory(int country);
void handleQuiz(int country);
void turnOffAllLEDs();
void playSimulatedAudio(int country);
void changeMode();
void showModeScreen(const char *modeName);

// =====================================================
// OLED SETTINGS
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET);

// =====================================================
// PIN DEFINITIONS
// =====================================================

// I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Mode button
#define MODE_BUTTON 27

// Country buttons
// 0 = India
// 1 = USA
// 2 = Brazil
// 3 = Africa
// 4 = Australia
// 5 = France

const int countryButtons[6] = {
    32,
    33,
    25,
    26,
    14,
    13};

// Country LEDs
const int countryLEDs[6] = {
    4,
    5,
    18,
    19,
    23,
    15};

// Buzzer
#define BUZZER 2

// =====================================================
// COUNTRY INFORMATION
// =====================================================

const char *countries[6] = {
    "India",
    "USA",
    "Brazil",
    "Africa",
    "Australia",
    "France"};

// =====================================================
// MODES
// =====================================================

enum Mode
{
  FACTS,
  STORY,
  QUIZ
};

Mode currentMode = FACTS;

// =====================================================
// QUIZ VARIABLES
// =====================================================

int quizScore = 0;
int quizQuestion = 0;

const char *quizQuestions[5] = {
    "Capital of India?",
    "Capital of France?",
    "Largest country?",
    "Brazil is in?",
    "Australia is a?"};

const char *quizAnswers[5] = {
    "New Delhi",
    "Paris",
    "Russia",
    "South America",
    "Country"};

// =====================================================
// DEBOUNCE VARIABLES
// =====================================================

unsigned long lastModePress = 0;
unsigned long lastCountryPress = 0;

const unsigned long debounceDelay = 250;

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("==============================");
  Serial.println("      TALKING GLOBE");
  Serial.println("      WOKWI VERSION 1");
  Serial.println("==============================");
  Serial.println();

  // -------------------------------------------------
  // Start I2C
  // -------------------------------------------------

  Wire.begin(SDA_PIN, SCL_PIN);

  // -------------------------------------------------
  // Start OLED
  // -------------------------------------------------

  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          OLED_ADDRESS))
  {
    Serial.println("ERROR: OLED not found!");

    while (true)
    {
      delay(1000);
    }
  }

  // -------------------------------------------------
  // Country buttons
  // -------------------------------------------------

  for (int i = 0; i < 6; i++)
  {
    pinMode(
        countryButtons[i],
        INPUT_PULLUP);
  }

  // -------------------------------------------------
  // Mode button
  // -------------------------------------------------

  pinMode(
      MODE_BUTTON,
      INPUT_PULLUP);

  // -------------------------------------------------
  // Country LEDs
  // -------------------------------------------------

  for (int i = 0; i < 6; i++)
  {
    pinMode(
        countryLEDs[i],
        OUTPUT);

    digitalWrite(
        countryLEDs[i],
        LOW);
  }

  // -------------------------------------------------
  // Buzzer
  // -------------------------------------------------

  pinMode(
      BUZZER,
      OUTPUT);

  // -------------------------------------------------
  // Initial screen
  // -------------------------------------------------

  showWelcomeScreen();

  Serial.println("Talking Globe ready!");
  Serial.println("Current mode: FACTS");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  checkCountryButtons();

  checkModeButton();

  delay(10);
}

// =====================================================
// WELCOME SCREEN
// =====================================================

void showWelcomeScreen()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(12, 5);
  display.println("TALKING");

  display.setCursor(22, 30);
  display.println("GLOBE");

  display.setTextSize(1);

  display.setCursor(25, 52);
  display.println("WOKWI V1");

  display.display();
}

// =====================================================
// CHECK COUNTRY BUTTONS
// =====================================================

void checkCountryButtons()
{
  for (int i = 0; i < 6; i++)
  {
    if (
        digitalRead(countryButtons[i]) == LOW)
    {
      unsigned long now = millis();

      if (
          now - lastCountryPress >
          debounceDelay)
      {
        lastCountryPress = now;

        handleCountry(i);
      }
    }
  }
}

// =====================================================
// HANDLE COUNTRY
// =====================================================

void handleCountry(int country)
{
  Serial.println();
  Serial.println("------------------------------");

  Serial.print("Country selected: ");
  Serial.println(countries[country]);

  Serial.print("Current mode: ");

  if (currentMode == FACTS)
  {
    Serial.println("FACTS");

    showFacts(country);
  }
  else if (currentMode == STORY)
  {
    Serial.println("STORY");

    showStory(country);
  }
  else if (currentMode == QUIZ)
  {
    Serial.println("QUIZ");

    handleQuiz(country);
  }

  Serial.println("------------------------------");
}

// =====================================================
// SHOW FACTS
// =====================================================

void showFacts(int country)
{
  turnOffAllLEDs();

  digitalWrite(
      countryLEDs[country],
      HIGH);

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("COUNTRY DETECTED");

  display.setTextSize(2);

  display.setCursor(0, 20);

  display.println(
      countries[country]);

  display.setTextSize(1);

  display.setCursor(0, 50);

  display.println("FACTS MODE");

  display.display();

  // Simulated audio
  playSimulatedAudio(country);
}

// =====================================================
// SHOW STORY
// =====================================================

void showStory(int country)
{
  turnOffAllLEDs();

  digitalWrite(
      countryLEDs[country],
      HIGH);

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("STORY MODE");

  display.setTextSize(2);

  display.setCursor(0, 20);

  display.println(
      countries[country]);

  display.setTextSize(1);

  display.setCursor(0, 50);

  display.println("Playing story...");

  display.display();

  playSimulatedAudio(country);
}

// =====================================================
// MODE BUTTON
// =====================================================

void checkModeButton()
{
  if (
      digitalRead(MODE_BUTTON) == LOW)
  {
    unsigned long now = millis();

    if (
        now - lastModePress >
        debounceDelay)
    {
      lastModePress = now;

      changeMode();
    }
  }
}

// =====================================================
// CHANGE MODE
// =====================================================

void changeMode()
{
  if (currentMode == FACTS)
  {
    currentMode = STORY;
  }
  else if (currentMode == STORY)
  {
    currentMode = QUIZ;
  }
  else
  {
    currentMode = FACTS;

    quizScore = 0;
    quizQuestion = 0;
  }

  Serial.println();
  Serial.println("==============================");

  Serial.print("MODE CHANGED TO: ");

  if (currentMode == FACTS)
  {
    Serial.println("FACTS");
    showModeScreen("FACTS");
  }
  else if (currentMode == STORY)
  {
    Serial.println("STORY");
    showModeScreen("STORY");
  }
  else
  {
    Serial.println("QUIZ");
    showModeScreen("QUIZ");
  }

  Serial.println("==============================");

  tone(
      BUZZER,
      1500,
      150);
}

// =====================================================
// MODE SCREEN
// =====================================================

void showModeScreen(const char *mode)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("MODE CHANGED");

  display.setTextSize(2);

  display.setCursor(0, 25);

  display.println(mode);

  display.display();
}

// =====================================================
// QUIZ
// =====================================================

void handleQuiz(int selectedCountry)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("QUIZ");

  display.setCursor(0, 15);

  display.print("Q");
  display.print(quizQuestion + 1);
  display.print(": ");

  display.println(
      quizQuestions[quizQuestion]);

  display.setCursor(0, 45);

  display.print("Score: ");
  display.print(quizScore);

  display.display();

  Serial.print("Question: ");
  Serial.println(
      quizQuestions[quizQuestion]);

  Serial.print("Selected: ");
  Serial.println(
      countries[selectedCountry]);

  quizQuestion++;

  if (quizQuestion >= 5)
  {
    quizQuestion = 0;

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);

    display.setCursor(0, 10);

    display.println("QUIZ COMPLETE");

    display.setTextSize(2);

    display.setCursor(20, 30);

    display.print("Score:");

    display.print(quizScore);

    display.display();

    Serial.println("Quiz completed.");

    delay(1500);
  }
}

// =====================================================
// SIMULATED AUDIO
// =====================================================

void playSimulatedAudio(int country)
{
  Serial.print("Playing audio for: ");

  Serial.println(
      countries[country]);

  Serial.println(
      "(DFPlayer simulated by buzzer)");

  tone(
      BUZZER,
      1000,
      250);
}

// =====================================================
// TURN OFF ALL LEDS
// =====================================================

void turnOffAllLEDs()
{
  for (int i = 0; i < 6; i++)
  {
    digitalWrite(
        countryLEDs[i],
        LOW);
  }
}