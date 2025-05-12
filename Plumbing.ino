// Pin configuration
const int LED_PIN = 49;
const int BUZZER_PIN = 35;
const int VIBRATION_PIN = 25;
const int HALL_SENSOR_PIN = A3;  // Analog hall sensor input

// Threshold for detecting significant change
const int HALL_DELTA_THRESHOLD = 50;  // Adjust based on your sensor's sensitivity

enum GameState {
  WAITING_FOR_PLUG,
  PLUGGING,
  PLUGGED
};

GameState state = WAITING_FOR_PLUG;
unsigned long stateStartTime = 0;
unsigned char playingVictory = false;

bool isPlugged = false;
unsigned char baselineHallValue = '\0';

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  pinMode(HALL_SENSOR_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Game started.");

  // Establish initial baseline (could also average multiple readings)
  baselineHallValue = analogRead(HALL_SENSOR_PIN);
  Serial.print("Baseline Hall value: ");
  Serial.println(baselineHallValue);
}

void loop() {
  unsigned char hallValue = analogRead(HALL_SENSOR_PIN);
  int delta = abs(hallValue - baselineHallValue);
  if (baselineHallValue != '\0' && delta > HALL_DELTA_THRESHOLD)
  {
    isPlugged = !isPlugged;
  }
  baselineHallValue = hallValue;

  Serial.print("Hall sensor value: ");
  Serial.print(hallValue);
  Serial.print(" | Δ: ");
  Serial.println(delta);
  if (isPlugged) Serial.println("PLUGGED!");

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
        Serial.println("Resetting baseline.");
      } else if (now - stateStartTime >= 2000) {
        Serial.println("Hole plugged successfully!");
        state = PLUGGED;
        stateStartTime = now;
        playingVictory = true;
        isPlugged = false;
      }
      break;
    }

    case PLUGGED: {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(VIBRATION_PIN, LOW);

      if ((now - stateStartTime) >= 300 && playingVictory) {
        noTone(BUZZER_PIN);
        playingVictory = false;
      } else if ((now - stateStartTime) >= 200 && playingVictory) {
        tone(BUZZER_PIN, 1000);
      } else if ((now - stateStartTime) >= 100 && playingVictory) {
        noTone(BUZZER_PIN);
      } else if (playingVictory) {
        tone(BUZZER_PIN, 1000);
      }

      if (now - stateStartTime >= 10000) {
        Serial.println("Restarting game.");
        noTone(BUZZER_PIN);
        Serial.println("New baseline recorded.");
        isPlugged = false;
        state = WAITING_FOR_PLUG;
      }
      break;
    }
  }
}