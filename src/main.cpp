#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// OLED DISPLAY SETTINGS
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

// =====================================================
// PIN DEFINITIONS
// =====================================================

// I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Mode Button
#define MODE_BUTTON 27

// Country Buttons (12 countries)
#define NUM_COUNTRIES 12

const int countryButtons[NUM_COUNTRIES] = {
    32, // 0: India
    33, // 1: USA
    25, // 2: Brazil
    26, // 3: Egypt
    14, // 4: Australia
    13, // 5: France
    16, // 6: Japan  (RX2)
    17, // 7: Canada (TX2)
    39, // 8: Mexico (VN)
    12, // 9: Italy
    34, // 10: Kenya
    35  // 11: Greece
};

// Country LEDs (6 wired in diagram)
#define NUM_LEDS 6
const int countryLEDs[NUM_LEDS] = {
    4,  // India
    5,  // USA
    18, // Brazil
    19, // Egypt
    23, // Australia
    15  // France
};

// =====================================================
// COUNTRY DATA STRUCTURE
// =====================================================

struct CountryInfo
{
    const char *name;
    const char *continent;
    const char *fact;
    const char *story;
    const char *quizQuestion;
};

const CountryInfo countryInfo[NUM_COUNTRIES] = {
    {
        "India",
        "Asia",
        "Home to the iconic Taj Mahal, built of white marble in Agra.",
        "Diwali, the festival of lights, fills homes with oil lamps and sweets.",
        "Which country is home to the Taj Mahal in Agra?"
    },
    {
        "USA",
        "North America",
        "A diverse nation spanning 50 states from the Atlantic to Pacific.",
        "Apollo 11 astronauts lifted off here and reached the Moon in 1969.",
        "Which nation is made of 50 states and sent Apollo to the Moon?"
    },
    {
        "Brazil",
        "South America",
        "Home to the Amazon, the world's largest tropical rainforest.",
        "Carnival brings vibrant parades, costumes, and samba to Rio.",
        "Which country is home to Rio and the vast Amazon rainforest?"
    },
    {
        "Egypt",
        "Africa",
        "The Great Pyramids and Sphinx stand tall along the River Nile.",
        "Ancient scribes wrote history in hieroglyphs on papyrus scrolls.",
        "Where are the Great Pyramids and the River Nile located?"
    },
    {
        "Australia",
        "Oceania",
        "Famous for the Great Barrier Reef and unique native wildlife.",
        "A rescued baby kangaroo (joey) grows safely in its mother's pouch.",
        "Which country is home to kangaroos & the Great Barrier Reef?"
    },
    {
        "France",
        "Europe",
        "The Eiffel Tower in Paris stands 330 meters above the River Seine.",
        "World-famous artists and writers met in the cafes of Montmartre.",
        "Which country is home to Paris and the Eiffel Tower?"
    },
    {
        "Japan",
        "Asia",
        "An island nation known for bullet trains, anime, and Mount Fuji.",
        "Cherry blossoms (Sakura) bloom in spring, celebrated with picnics.",
        "Land of Mount Fuji, bullet trains, and cherry blossoms?"
    },
    {
        "Canada",
        "North America",
        "The second largest country by area, with the world's longest coast.",
        "Park rangers watch over grizzly bears in the snowy Rocky Mountains.",
        "Which country has the world's longest coastline & Rockies?"
    },
    {
        "Mexico",
        "North America",
        "Rich in history, from ancient Mayan pyramids to vibrant cuisine.",
        "Dia de los Muertos honors loved ancestors with marigold flowers.",
        "Famous for ancient Mayan pyramids and Dia de los Muertos?"
    },
    {
        "Italy",
        "Europe",
        "Historic heart of the Roman Empire, delicious pizza, and gelato.",
        "Gondolas carry passengers through the winding canals of Venice.",
        "Which country is home to Rome, the Colosseum, and Venice?"
    },
    {
        "Kenya",
        "Africa",
        "World-renowned for Mount Kenya and the Maasai Mara national reserve.",
        "Over a million wildebeest migrate across the Mara River each year.",
        "Which African nation is famed for the Maasai Mara safari?"
    },
    {
        "Greece",
        "Europe",
        "The cradle of Western democracy, philosophy, and Olympic Games.",
        "The ancient Olympic Games began here in Olympia over 2,700 yrs ago.",
        "Where were the ancient Olympic Games and democracy born?"
    }
};

// =====================================================
// OPERATING MODES & STATE
// =====================================================

enum Mode
{
    FACTS,
    STORY,
    QUIZ
};

Mode currentMode = FACTS;

// Quiz state & randomization
int quizOrder[NUM_COUNTRIES];
int quizIndex = 0;
int quizScore = 0;
bool quizAnswerPending = false;

// Button debounce tracking
bool countryButtonWasPressed[NUM_COUNTRIES] = {};
unsigned long lastCountryPressTime[NUM_COUNTRIES] = {};

bool modeButtonWasPressed = false;
unsigned long lastModePressTime = 0;
const unsigned long DEBOUNCE_DELAY = 200; // ms

// =====================================================
// FUNCTION PROTOTYPES
// =====================================================

void showWelcomeScreen();
void showModeIntro(Mode mode);
void displayFacts(int country);
void displayStory(int country);

void shuffleQuiz();
void displayQuizQuestion();
void handleQuizAnswer(int selectedCountry);
void finishQuiz();

void checkCountryButtons();
void checkModeButton();
void changeMode();
void turnOffAllLEDs();
void highlightLED(int country);

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println();
    Serial.println("========================================");
    Serial.println("         TALKING GLOBE ESP32            ");
    Serial.println("========================================");

    // Seed the hardware random number generator
    randomSeed(esp_random());

    // Initialize I2C and OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("ERROR: SSD1306 OLED not detected!");
        while (true)
        {
            delay(1000);
        }
    }

    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(true);

    // Initialize LEDs
    for (int i = 0; i < NUM_LEDS; i++)
    {
        pinMode(countryLEDs[i], OUTPUT);
        digitalWrite(countryLEDs[i], LOW);
    }

    // Initialize Mode Button
    pinMode(MODE_BUTTON, INPUT_PULLUP);
    modeButtonWasPressed = (digitalRead(MODE_BUTTON) == LOW);

    // Initialize Country Buttons
    // Pins 34, 35, 39 are input-only without internal pullups; use INPUT
    for (int i = 0; i < NUM_COUNTRIES; i++)
    {
        const int pin = countryButtons[i];
        const bool inputOnly = (pin == 34 || pin == 35 || pin == 39);

        pinMode(pin, inputOnly ? INPUT : INPUT_PULLUP);
        countryButtonWasPressed[i] = (digitalRead(pin) == LOW);
        lastCountryPressTime[i] = 0;
    }

    // Prepare initial random quiz order
    shuffleQuiz();

    // Show Welcome Splash
    showWelcomeScreen();
    delay(1800);

    // Transition to default FACTS mode
    showModeIntro(FACTS);
    Serial.println("Talking Globe ready in FACTS mode.");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    checkModeButton();
    checkCountryButtons();
    delay(10);
}

// =====================================================
// SCREEN RENDERING
// =====================================================

void showWelcomeScreen()
{
    display.clearDisplay();
    display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(20, 8);
    display.print("TALKING");

    display.setCursor(32, 28);
    display.print("GLOBE");

    display.setTextSize(1);
    display.setCursor(18, 48);
    display.print("Press Any Button");
    display.display();
}

void showModeIntro(Mode mode)
{
    turnOffAllLEDs();
    display.clearDisplay();
    display.drawFastHLine(0, 14, 128, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);

    if (mode == FACTS)
    {
        display.print("MODE: COUNTRY FACTS");
        display.setCursor(2, 20);
        display.println("Press any country");
        display.println("button to learn cool");
        display.println("geography facts!");
    }
    else if (mode == STORY)
    {
        display.print("MODE: CULTURE & TALES");
        display.setCursor(2, 20);
        display.println("Press any country");
        display.println("button to discover");
        display.println("stories and folklore!");
    }
    else if (mode == QUIZ)
    {
        display.print("MODE: GLOBE QUIZ");
        display.setCursor(2, 20);
        display.println("Get ready!");
        display.println("Answer questions by");
        display.println("pressing country keys.");
    }

    display.display();
}

void displayFacts(int country)
{
    highlightLED(country);

    display.clearDisplay();
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("FACTS: ");
    display.print(countryInfo[country].name);

    display.setCursor(2, 16);
    display.println(countryInfo[country].fact);

    display.setCursor(2, 54);
    display.print("Region: ");
    display.print(countryInfo[country].continent);

    display.display();

    Serial.println();
    Serial.println("--- [FACTS] ---");
    Serial.print("Country:   "); Serial.println(countryInfo[country].name);
    Serial.print("Continent: "); Serial.println(countryInfo[country].continent);
    Serial.print("Fact:      "); Serial.println(countryInfo[country].fact);
}

void displayStory(int country)
{
    highlightLED(country);

    display.clearDisplay();
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("STORY: ");
    display.print(countryInfo[country].name);

    display.setCursor(2, 16);
    display.println(countryInfo[country].story);

    display.setCursor(2, 54);
    display.print("Region: ");
    display.print(countryInfo[country].continent);

    display.display();

    Serial.println();
    Serial.println("--- [STORY] ---");
    Serial.print("Country:   "); Serial.println(countryInfo[country].name);
    Serial.print("Story:     "); Serial.println(countryInfo[country].story);
}

// =====================================================
// RANDOMIZED QUIZ SYSTEM
// =====================================================

void shuffleQuiz()
{
    // Populate with 0..NUM_COUNTRIES-1
    for (int i = 0; i < NUM_COUNTRIES; i++)
    {
        quizOrder[i] = i;
    }

    // Fisher-Yates shuffle using hardware random
    for (int i = NUM_COUNTRIES - 1; i > 0; i--)
    {
        int j = esp_random() % (i + 1);
        int temp = quizOrder[i];
        quizOrder[i] = quizOrder[j];
        quizOrder[j] = temp;
    }

    Serial.print("New randomized quiz order: ");
    for (int i = 0; i < NUM_COUNTRIES; i++)
    {
        Serial.print(countryInfo[quizOrder[i]].name);
        if (i < NUM_COUNTRIES - 1) Serial.print(", ");
    }
    Serial.println();
}

void displayQuizQuestion()
{
    turnOffAllLEDs();
    quizAnswerPending = true;

    const int targetCountry = quizOrder[quizIndex];

    display.clearDisplay();
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Q");
    display.print(quizIndex + 1);
    display.print("/12  Score:");
    display.print(quizScore);

    display.setCursor(2, 16);
    display.println(countryInfo[targetCountry].quizQuestion);

    display.display();

    Serial.println();
    Serial.print("--- [QUIZ Q"); Serial.print(quizIndex + 1); Serial.println("/12] ---");
    Serial.println(countryInfo[targetCountry].quizQuestion);
    Serial.println("Press the matching country button!");
}

void handleQuizAnswer(int selectedCountry)
{
    quizAnswerPending = false;
    const int targetCountry = quizOrder[quizIndex];
    const bool isCorrect = (selectedCountry == targetCountry);

    display.clearDisplay();
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(2, 2);
    display.print("Q");
    display.print(quizIndex + 1);
    display.print("/12 Result");

    if (isCorrect)
    {
        quizScore++;
        highlightLED(selectedCountry);

        display.setTextSize(2);
        display.setCursor(18, 18);
        display.print("CORRECT!");

        display.setTextSize(1);
        display.setCursor(16, 38);
        display.print(countryInfo[selectedCountry].name);
        display.print(" (+1 pt)");

        display.setCursor(16, 52);
        display.print("Score: ");
        display.print(quizScore);
        display.print("/12");

        display.display();

        Serial.println("RESULT: CORRECT! Well done.");
    }
    else
    {
        display.setTextSize(2);
        display.setCursor(8, 18);
        display.print("INCORRECT!");

        display.setTextSize(1);
        display.setCursor(2, 38);
        display.print("Ans: ");
        display.print(countryInfo[targetCountry].name);

        display.setCursor(2, 52);
        display.print("Picked: ");
        display.print(countryInfo[selectedCountry].name);

        display.display();

        Serial.print("RESULT: INCORRECT. You pressed: ");
        Serial.print(countryInfo[selectedCountry].name);
        Serial.print(", correct answer was: ");
        Serial.println(countryInfo[targetCountry].name);
    }

    delay(1600);

    quizIndex++;
    if (quizIndex >= NUM_COUNTRIES)
    {
        finishQuiz();
    }
    else
    {
        displayQuizQuestion();
    }
}

void finishQuiz()
{
    turnOffAllLEDs();

    display.clearDisplay();
    display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(20, 8);
    display.print("QUIZ COMPLETE!");

    display.setTextSize(2);
    display.setCursor(30, 24);
    display.print(quizScore);
    display.print("/12");

    display.setTextSize(1);
    display.setCursor(14, 46);
    if (quizScore >= 10)
    {
        display.print("Master Geographer!");
    }
    else if (quizScore >= 6)
    {
        display.print("Great Explorer!");
    }
    else
    {
        display.print("Keep Exploring!");
    }
    display.display();

    Serial.println();
    Serial.println("========================================");
    Serial.print("QUIZ OVER! Final Score: ");
    Serial.print(quizScore);
    Serial.println("/12");
    Serial.println("========================================");

    delay(2800);

    // Reshuffle for next game
    shuffleQuiz();
    quizIndex = 0;
    quizScore = 0;
    displayQuizQuestion();
}

// =====================================================
// HARDWARE CONTROLS (BUTTONS & LEDS)
// =====================================================

void checkCountryButtons()
{
    unsigned long now = millis();

    for (int i = 0; i < NUM_COUNTRIES; i++)
    {
        bool isPressed = (digitalRead(countryButtons[i]) == LOW);

        if (isPressed && !countryButtonWasPressed[i])
        {
            if (now - lastCountryPressTime[i] > DEBOUNCE_DELAY)
            {
                lastCountryPressTime[i] = now;

                if (currentMode == FACTS)
                {
                    displayFacts(i);
                }
                else if (currentMode == STORY)
                {
                    displayStory(i);
                }
                else if (currentMode == QUIZ)
                {
                    if (quizAnswerPending)
                    {
                        handleQuizAnswer(i);
                    }
                }
            }
        }
        countryButtonWasPressed[i] = isPressed;
    }
}

void checkModeButton()
{
    unsigned long now = millis();
    bool isPressed = (digitalRead(MODE_BUTTON) == LOW);

    if (isPressed && !modeButtonWasPressed)
    {
        if (now - lastModePressTime > DEBOUNCE_DELAY)
        {
            lastModePressTime = now;
            changeMode();
        }
    }
    modeButtonWasPressed = isPressed;
}

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
    }

    Serial.println();
    Serial.println("========================================");
    Serial.print("MODE SWITCHED TO: ");
    if (currentMode == FACTS) Serial.println("FACTS");
    else if (currentMode == STORY) Serial.println("STORY");
    else if (currentMode == QUIZ) Serial.println("QUIZ");
    Serial.println("========================================");

    if (currentMode == QUIZ)
    {
        shuffleQuiz();
        quizIndex = 0;
        quizScore = 0;
        showModeIntro(QUIZ);
        delay(1200);
        displayQuizQuestion();
    }
    else
    {
        showModeIntro(currentMode);
    }
}

void turnOffAllLEDs()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        digitalWrite(countryLEDs[i], LOW);
    }
}

void highlightLED(int country)
{
    turnOffAllLEDs();
    if (country < NUM_LEDS)
    {
        digitalWrite(countryLEDs[country], HIGH);
    }
}