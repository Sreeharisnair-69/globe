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
const int countryButtons[12] = {
    32,
    33,
    25,
    26,
    14,
    13,
    16,
    17,
    39,
    12,
    34,
    35};

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

struct CountryInfo
{
  const char *name;
  const char *fact;
  const char *story;
  const char *question;
  const char *answer;
};

const CountryInfo countryInfo[12] = {
    {"India", "The Taj Mahal is in Agra.", "Diwali fills homes with lamps.", "What is India's capital?", "New Delhi"},
    {"USA", "The USA has 50 states.", "Apollo 11 reached the Moon in 1969.", "What is the USA's capital?", "Washington, D.C."},
    {"Brazil", "Brazil is home to the Amazon.", "Carnival brings music to Rio.", "Which continent is Brazil in?", "South America"},
    {"Egypt", "The pyramids stand at Giza.", "Ancient scribes wrote on papyrus.", "What river runs through Egypt?", "The Nile"},
    {"Australia", "Australia has the Great Barrier Reef.", "A rescued joey grows in a pouch.", "What is Australia's capital?", "Canberra"},
    {"France", "The Eiffel Tower is in Paris.", "Artists once gathered in Montmartre.", "What is France's capital?", "Paris"},
    {"Japan", "Japan is made of thousands of islands.", "Cherry blossoms mark the spring.", "What is Japan's capital?", "Tokyo"},
    {"Canada", "Canada has the world's longest coastline.", "Rangers guide visitors through the Rockies.", "What is Canada's capital?", "Ottawa"},
    {"Mexico", "Mexico is famous for ancient Maya cities.", "Day of the Dead honors loved ones.", "What is Mexico's capital?", "Mexico City"},
    {"Italy", "Rome is home to the Colosseum.", "Venice is built around canals.", "What is Italy's capital?", "Rome"},
    {"Kenya", "Kenya is known for the Maasai Mara.", "Wildebeest cross the Mara each year.", "What is Kenya's capital?", "Nairobi"},
    {"Greece", "The Acropolis overlooks Athens.", "The first Olympic Games began in Greece.", "What is Greece's capital?", "Athens"}};

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

// =====================================================
// DEBOUNCE VARIABLES
// =====================================================

unsigned long lastModePress = 0;
unsigned long lastCountryPress = 0;
bool countryButtonWasPressed[12] = {};

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

  for (int i = 0; i < 12; i++)
  {
    pinMode(
        countryButtons[i],
        i >= 6 && i != 9 ? INPUT : INPUT_PULLUP);

    countryButtonWasPressed[i] =
        digitalRead(countryButtons[i]) == LOW;
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
  for (int i = 0; i < 12; i++)
  {
    bool buttonPressed =
        digitalRead(countryButtons[i]) == LOW;

    if (
        buttonPressed && !countryButtonWasPressed[i])
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

    countryButtonWasPressed[i] = buttonPressed;
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
  Serial.println(countryInfo[country].name);

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

  if (country < 6)
  {
    digitalWrite(
        countryLEDs[country],
        HIGH);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("COUNTRY DETECTED");

  display.setCursor(0, 15);

  display.println(countryInfo[country].name);

  display.setCursor(0, 30);

  display.println(countryInfo[country].fact);

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

  if (country < 6)
  {
    digitalWrite(
        countryLEDs[country],
        HIGH);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.println("STORY MODE");

  display.setCursor(0, 15);

  display.println(countryInfo[country].name);

  display.setCursor(0, 30);

  display.println(countryInfo[country].story);

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

  display.println(countryInfo[selectedCountry].question);

  display.setCursor(0, 45);

  display.print("Score: ");
  display.print(quizScore);

  display.display();

  Serial.print("Question: ");
  Serial.println(countryInfo[selectedCountry].question);

  Serial.print("Answer: ");
  Serial.println(countryInfo[selectedCountry].answer);

  Serial.print("Selected: ");
  Serial.println(
      countryInfo[selectedCountry].name);

  quizQuestion++;

  if (quizQuestion >= 12)
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

  Serial.println(countryInfo[country].name);

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