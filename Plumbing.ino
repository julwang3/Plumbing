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

// --- Constants ---
const int HALL_DELTA_THRESHOLD = 50;  // Sensitivity threshold

// --- Game States ---
enum GameState {
  WAITING_TO_START,
  WAITING_FOR_PLUG,
  PLUGGING,
  PLUGGED
};

// --- Pipe Game Structure with Constructor ---
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
    // pinMode(buzzerPin, OUTPUT);
    // pinMode(vibrationPin, OUTPUT);
    pinMode(hallPin, INPUT_PULLUP);

    cooldownStartTime = millis();
    cooldownDuration = random(3000, 12000);

    Serial.print("Cooldown for pin ");
    Serial.print(hallPin);
    Serial.print(": ");
    Serial.println(cooldownDuration);
  }

  void update(const char* label) {
    unsigned char hallValue = analogRead(hallPin);
    int delta = abs(hallValue - baselineHallValue);

    if (baselineHallValue != '\0' && delta > HALL_DELTA_THRESHOLD) {
      isPlugged = !isPlugged;
    }
    baselineHallValue = hallValue;

    unsigned long now = millis();

    Serial.print(label);
    Serial.print(" | Hall: ");
    Serial.print(hallValue);
    Serial.print(" | Δ: ");
    Serial.print(delta);
    Serial.print(" | Plugged: ");
    Serial.println(isPlugged ? "YES" : "NO");

    switch (state) {
      case WAITING_TO_START:
        digitalWrite(ledPin, LOW);
        // digitalWrite(buzzerPin, LOW);
        // digitalWrite(vibrationPin, LOW);
        if (now - cooldownStartTime >= cooldownDuration) {
          baselineHallValue = analogRead(hallPin);
          stateStartTime = now;
          state = WAITING_FOR_PLUG;
          Serial.print(label); Serial.println(" is READY!");
        }
        break;

      case WAITING_FOR_PLUG:
        // digitalWrite(vibrationPin, HIGH);
        digitalWrite(ledPin, (now % 1000 < 100) ? HIGH : LOW);
        // (now % 1000 < 100) ? tone(buzzerPin, 1000) : noTone(buzzerPin);
        if (isPlugged) {
          state = PLUGGING;
          stateStartTime = now;
          Serial.print(label); Serial.println(" plugging started.");
        }
        break;

      case PLUGGING:
        digitalWrite(ledPin, HIGH);
        // digitalWrite(vibrationPin, HIGH);
        // noTone(buzzerPin);
        if (!isPlugged) {
          Serial.print(label); Serial.println(" plug removed too early.");
          state = WAITING_FOR_PLUG;
        } else if (now - stateStartTime >= 2000) {
          Serial.print(label); Serial.println(" plug SUCCESSFUL!");
          state = PLUGGED;
          stateStartTime = now;
          playingVictory = true;
          isPlugged = false;
        }
        break;

      case PLUGGED:
        digitalWrite(ledPin, LOW);
        // digitalWrite(vibrationPin, LOW);

        if ((now - stateStartTime) >= 300 && playingVictory) {
          // noTone(buzzerPin);
          playingVictory = false;
        } else if ((now - stateStartTime) >= 200 && playingVictory) {
          // tone(buzzerPin, 1000);
        } else if ((now - stateStartTime) >= 100 && playingVictory) {
          // noTone(buzzerPin);
        } else if (playingVictory) {
          // tone(buzzerPin, 1000);
        }

        if (now - stateStartTime >= 10000) {
          Serial.print(label); Serial.println(" restarting.");
          // noTone(buzzerPin);
          isPlugged = false;
          state = WAITING_TO_START;
          cooldownStartTime = millis();
          cooldownDuration = random(4000, 12000);
        }
        break;
    }
  }
};

// --- Pipe Instances ---
PipeGame pipe1(LED_PIN_1, BUZZER_PIN_1, VIBRATION_PIN_1, HALL_SENSOR_PIN_1);
PipeGame pipe2(LED_PIN_2, BUZZER_PIN_2, VIBRATION_PIN_2, HALL_SENSOR_PIN_2);

// --- Arduino Setup ---
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  pipe1.setupPins();
  pipe2.setupPins();
  Serial.println("Two-pipe game initialized.");
}

// --- Main Loop ---
void loop() {
  pipe1.update("Pipe 1");
  pipe2.update("Pipe 2");
}
