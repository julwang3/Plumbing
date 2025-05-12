// Pin configuration
const int LED_PIN = 52;
const int BUZZER_PIN = 38;
const int VIBRATION_PIN = 28;
const int HALL_SENSOR_PIN = A6;  // Analog hall sensor input

// Hysteresis thresholds (can be tuned)
const unsigned char HALL_THRESHOLD_LOW = 50;
const unsigned char HALL_THRESHOLD_HIGH = 200;

enum GameState {
  WAITING_FOR_PLUG,
  PLUGGING,
  PLUGGED
};

GameState state = WAITING_FOR_PLUG;
unsigned long stateStartTime = 0;
unsigned char playingVictory = false;
bool isPlugged = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  pinMode(HALL_SENSOR_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Game started.");
}

void loop() {
  unsigned char hallValue = analogRead(HALL_SENSOR_PIN);
  Serial.print("Hall sensor value: ");
  Serial.println(hallValue);

  // Update isPlugged using hysteresis
  if (isPlugged) {
    // Once plugged, stay plugged until HIGH threshold is exceeded
    isPlugged = (HALL_THRESHOLD_LOW < HALL_THRESHOLD_HIGH)
                ? (hallValue < HALL_THRESHOLD_HIGH)
                : (hallValue > HALL_THRESHOLD_HIGH);
  } else {
    // Once unplugged, stay unplugged until LOW threshold is crossed
    isPlugged = (HALL_THRESHOLD_LOW < HALL_THRESHOLD_HIGH)
                ? (hallValue < HALL_THRESHOLD_LOW)
                : (hallValue > HALL_THRESHOLD_LOW);
  }

  unsigned long now = millis();

  switch (state) {
    case WAITING_FOR_PLUG: {
      digitalWrite(VIBRATION_PIN, HIGH);
      unsigned long blinkPhase = now % 1000;
      bool blink = blinkPhase < 100;

      digitalWrite(LED_PIN, blink);
      if (blink) {
        tone(BUZZER_PIN, 1000);
      } else {
        noTone(BUZZER_PIN);
      }

      if (isPlugged) {
        state = PLUGGING;
        stateStartTime = now;
        Serial.println("Started plugging...");
      }
      break;
    }

    case PLUGGING: {
      digitalWrite(LED_PIN, HIGH);
      noTone(BUZZER_PIN);
      digitalWrite(VIBRATION_PIN, HIGH);

      if (!isPlugged) {
        Serial.println("Plug removed too early.");
        state = WAITING_FOR_PLUG;
        noTone(BUZZER_PIN);
      } else if (now - stateStartTime >= 2000) {
        Serial.println("Hole plugged successfully!");
        state = PLUGGED;
        stateStartTime = now;
        playingVictory = true;
      }
      break;
    }

    case PLUGGED: {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(VIBRATION_PIN, LOW);

      unsigned long elapsed = now - stateStartTime;
      if (playingVictory) {
        if (elapsed < 100) {
          tone(BUZZER_PIN, 1000);
        } else if (elapsed < 200) {
          noTone(BUZZER_PIN);
        } else if (elapsed < 300) {
          tone(BUZZER_PIN, 1000);
        } else {
          noTone(BUZZER_PIN);
          playingVictory = false;
        }
      }

      if (elapsed >= 10000) {
        Serial.println("Restarting game.");
        noTone(BUZZER_PIN);
        state = WAITING_FOR_PLUG;
      }
      break;
    }
  }
}
