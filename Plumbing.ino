// --- Pin Configuration ---
// Pipe 1
const int LED_PIN_1 = 46;
const int BUZZER_PIN_1 = 32;
const int VIBRATION_PIN_1 = 22;
const int HALL_SENSOR_PIN_1 = A0;
// Pipe 2
const int LED_PIN_2 = 47;
const int BUZZER_PIN_2 = 33;
const int VIBRATION_PIN_2 = 23;
const int HALL_SENSOR_PIN_2 = A1;
// Pipe 3
const int LED_PIN_3 = 48;
const int BUZZER_PIN_3 = 34;
const int VIBRATION_PIN_3 = 24;
const int HALL_SENSOR_PIN_3 = A2;
// Pipe 5
const int LED_PIN_5 = 50;
const int BUZZER_PIN_5 = 36;
const int VIBRATION_PIN_5 = 26;
const int HALL_SENSOR_PIN_5 = A4;
// Pipe 6
const int LED_PIN_6 = 51;
const int BUZZER_PIN_6 = 37;
const int VIBRATION_PIN_6 = 27;
const int HALL_SENSOR_PIN_6 = A5;
// Pipe 7
const int LED_PIN_7 = 52;
const int BUZZER_PIN_7 = 38;
const int VIBRATION_PIN_7 = 28;
const int HALL_SENSOR_PIN_7 = A6;

// --- Constants ---
const int HALL_DELTA_THRESHOLD = 50;
const unsigned long GAME_DURATION = 60000;  // 1 minute in milliseconds
const unsigned long BLINK_INTERVAL = 100;   // Blink speed

// --- Global Variables ---
bool globalBlink = false;
unsigned long lastBlinkToggle = 0;
unsigned long gameStartTime = 0;
bool gameStopped = false;
int fixedLeaksCount = 0;  // ✅ Tracks number of successful fixes

// --- Static Variables ---
static bool timeIsUp = false;
static bool pendingEnd = false;

// --- Helper Functions ---
int getMaxAllowedActiveLeaks(unsigned long elapsedTime);
int getCurrentActiveLeaks();
bool allPipesIdle();

// --- Game States ---
enum GameState {
  WAITING_TO_START,
  WAITING_FOR_PLUG,
  PLUGGING,
  PLUGGED
};

// --- PipeGame Struct ---
struct PipeGame {
  int ledPin;
  int buzzerPin;
  int vibrationPin;
  int hallPin;

  GameState state;
  unsigned long stateStartTime;
  bool playingVictory;
  bool isPlugged;
  unsigned char baselineHallValue;

  unsigned long cooldownStartTime;
  unsigned long cooldownDuration;

  PipeGame(int led, int buzzer, int vibration, int hall)
    : ledPin(led), buzzerPin(buzzer), vibrationPin(vibration), hallPin(hall),
      state(WAITING_TO_START), stateStartTime(0), playingVictory(false),
      isPlugged(false), baselineHallValue('\0'), cooldownStartTime(0), cooldownDuration(0) {}

  void setupPins() {
    pinMode(ledPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(vibrationPin, OUTPUT);
    pinMode(hallPin, INPUT_PULLUP);
    cooldownStartTime = millis();
    cooldownDuration = random(2000, 10000); // Start with faster leak intervals
  }

  void update(const char* label, bool blinkState) {
    if (gameStopped) return;

    unsigned char hallValue = analogRead(hallPin);
    int delta = abs(hallValue - baselineHallValue);

    if (baselineHallValue != '\0' && delta > HALL_DELTA_THRESHOLD) {
      isPlugged = !isPlugged;
    }
    baselineHallValue = hallValue;

    unsigned long now = millis();

    switch (state) {
      case WAITING_TO_START:
        digitalWrite(ledPin, LOW);
        // digitalWrite(buzzerPin, LOW);
        // digitalWrite(vibrationPin, LOW);
        if (now - cooldownStartTime >= cooldownDuration) {
          int activeLeaks = getCurrentActiveLeaks();
          int allowedLeaks = getMaxAllowedActiveLeaks(now - gameStartTime);

          if (activeLeaks < allowedLeaks && !pendingEnd) {
            baselineHallValue = analogRead(hallPin);
            stateStartTime = now;
            state = WAITING_FOR_PLUG;
          } else {
            // Not yet allowed — check again in short while
            cooldownStartTime = now;
            cooldownDuration = random(500, 2000); // Shorter cooldown for higher intensity
          }
        }
        break;

      case WAITING_FOR_PLUG:
        // digitalWrite(vibrationPin, HIGH);
        digitalWrite(ledPin, blinkState ? HIGH : LOW);
        // blinkState ? tone(buzzerPin, 1000) : noTone(buzzerPin);
        if (isPlugged) {
          state = PLUGGING;
          stateStartTime = now;
        }
        break;

      case PLUGGING:
        digitalWrite(ledPin, HIGH);
        // digitalWrite(vibrationPin, HIGH);
        // noTone(buzzerPin);
        if (!isPlugged) {
          state = WAITING_FOR_PLUG;
        } else if (now - stateStartTime >= 1500) {
          state = PLUGGED;
          stateStartTime = now;
          playingVictory = true;
          isPlugged = false;
          fixedLeaksCount++;  // ✅ Count successful plug
        }
        break;

      case PLUGGED:
        digitalWrite(ledPin, LOW);
        // digitalWrite(vibrationPin, LOW);

        // if ((now - stateStartTime) >= 300 && playingVictory) {
        //   noTone(buzzerPin);
        //   playingVictory = false;
        // } else if ((now - stateStartTime) >= 200 && playingVictory) {
        //   tone(buzzerPin, 1000);
        // } else if ((now - stateStartTime) >= 100 && playingVictory) {
        //   noTone(buzzerPin);
        // } else if (playingVictory) {
        //   tone(buzzerPin, 1000);
        // }
        
        if (now - stateStartTime >= 5000) {
          isPlugged = false;
          state = WAITING_TO_START;
          cooldownStartTime = millis();
          cooldownDuration = random(2000, 10000); // Faster leaks
        }
        break;
    }
  }

  void forceLED(bool on) {
    digitalWrite(ledPin, on ? HIGH : LOW);
  }
};

// --- Pipe Instances ---
PipeGame pipe1(LED_PIN_1, BUZZER_PIN_1, VIBRATION_PIN_1, HALL_SENSOR_PIN_1);
PipeGame pipe2(LED_PIN_2, BUZZER_PIN_2, VIBRATION_PIN_2, HALL_SENSOR_PIN_2);
PipeGame pipe3(LED_PIN_3, BUZZER_PIN_3, VIBRATION_PIN_3, HALL_SENSOR_PIN_3);
PipeGame pipe5(LED_PIN_5, BUZZER_PIN_5, VIBRATION_PIN_5, HALL_SENSOR_PIN_5);
PipeGame pipe6(LED_PIN_6, BUZZER_PIN_6, VIBRATION_PIN_6, HALL_SENSOR_PIN_6);
PipeGame pipe7(LED_PIN_7, BUZZER_PIN_7, VIBRATION_PIN_7, HALL_SENSOR_PIN_7);

PipeGame* allPipes[] = { &pipe1, &pipe2, &pipe3, &pipe5, &pipe6, &pipe7 };
const int PIPE_COUNT = sizeof(allPipes) / sizeof(allPipes[0]);

int getMaxAllowedActiveLeaks(unsigned long elapsedTime) {
  // Ramp up difficulty faster by scaling exponentially over the first 30 seconds
  float progress = min(1.75f, (float)elapsedTime / (GAME_DURATION / 2.0f));

  // Exponential growth with faster intensity
  float exponent = 4.0f; // Increased exponent for faster difficulty ramp
  float maxLeaksProgress = pow(progress, exponent); // Apply exponential scaling

  // Scale it to the desired range [1, PIPE_COUNT]
  int maxLeaks = 1 + (int)(maxLeaksProgress * (PIPE_COUNT - 1));
  return maxLeaks;
}

int getCurrentActiveLeaks() {
  int count = 0;
  for (int i = 0; i < PIPE_COUNT; i++) {
    if (allPipes[i]->state == WAITING_FOR_PLUG || allPipes[i]->state == PLUGGING) {
      count++;
    }
  }
  return count;
}

bool allPipesIdle() {
  for (int i = 0; i < PIPE_COUNT; i++) {
    GameState s = allPipes[i]->state;
    if (s == WAITING_FOR_PLUG || s == PLUGGING || s == PLUGGED) {
      return false;
    }
  }
  return true;
}

// --- Setup ---
void setup() {
  Serial.begin(9600);
  randomSeed(millis());

  for (int i = 0; i < PIPE_COUNT; i++) {
    allPipes[i]->setupPins();
  }

  gameStartTime = millis();
  Serial.println("Game started.");
}

// --- Main Loop ---
void loop() {
  unsigned long now = millis();

  // Toggle global blink
  if (!gameStopped && (now - lastBlinkToggle >= BLINK_INTERVAL)) {
    globalBlink = !globalBlink;
    lastBlinkToggle = now;
  }

  if (!gameStopped) {
    // Run pipe updates
    for (int i = 0; i < PIPE_COUNT; i++) {
      allPipes[i]->update("Pipe", globalBlink);
    }

    // Handle game duration expiration
    if (!timeIsUp && (now - gameStartTime >= GAME_DURATION)) {
      timeIsUp = true;

      Serial.print("TIME'S UP! Total fixed leaks: ");
      Serial.println(fixedLeaksCount);

      if (fixedLeaksCount < 15) {
        Serial.println("Not enough leaks fixed — all LEDs ON.");
        for (int j = 0; j < PIPE_COUNT; j++) {
          allPipes[j]->forceLED(true);
        }
        gameStopped = true; // Immediate end
      } else {
        Serial.println("Enough leaks fixed — waiting for remaining leaks to be plugged...");
        pendingEnd = true;
        // Don't stop yet — wait for all leaks to be plugged
      }
    }

    // Handle delayed game stop after all leaks plugged
    if (timeIsUp && pendingEnd && allPipesIdle()) {
      Serial.println("All leaks plugged. Ending game.");
      for (int j = 0; j < PIPE_COUNT; j++) {
        allPipes[j]->forceLED(false);
      }
      gameStopped = true;
    }
  }
}

