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
// Pipe 4 doesn't work.
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
const int HALL_DELTA_THRESHOLD = 50;  // Sensitivity threshold

// --- Global Blink Timing ---
bool globalBlink = false;
unsigned long lastBlinkToggle = 0;
const unsigned long BLINK_INTERVAL = 100;  // Blink every 100 ms

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

  // ✅ Constructor
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
    cooldownDuration = random(3000, 30000);

    Serial.print("Cooldown for pin ");
    Serial.print(hallPin);
    Serial.print(": ");
    Serial.println(cooldownDuration);
  }

  void update(const char* label, bool blinkState) {
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
        digitalWrite(ledPin, blinkState ? HIGH : LOW);
        // blinkState ? tone(buzzerPin, 1000) : noTone(buzzerPin);
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

        if (now - stateStartTime >= 10000) {
          Serial.print(label); Serial.println(" restarting.");
          // noTone(buzzerPin);
          isPlugged = false;
          state = WAITING_TO_START;
          cooldownStartTime = millis();
          cooldownDuration = random(4000, 20000);
        }
        break;
    }
  }
};

// --- Pipe Instances ---
PipeGame pipe1(LED_PIN_1, BUZZER_PIN_1, VIBRATION_PIN_1, HALL_SENSOR_PIN_1);
PipeGame pipe2(LED_PIN_2, BUZZER_PIN_2, VIBRATION_PIN_2, HALL_SENSOR_PIN_2);
PipeGame pipe3(LED_PIN_3, BUZZER_PIN_3, VIBRATION_PIN_3, HALL_SENSOR_PIN_3);
PipeGame pipe5(LED_PIN_5, BUZZER_PIN_5, VIBRATION_PIN_5, HALL_SENSOR_PIN_5);
PipeGame pipe6(LED_PIN_6, BUZZER_PIN_6, VIBRATION_PIN_6, HALL_SENSOR_PIN_6);
PipeGame pipe7(LED_PIN_7, BUZZER_PIN_7, VIBRATION_PIN_7, HALL_SENSOR_PIN_7);

// --- Arduino Setup ---
void setup() {
  Serial.begin(9600);
  randomSeed(millis());
  pipe1.setupPins();
  pipe2.setupPins();
  pipe3.setupPins();
  pipe5.setupPins();
  pipe6.setupPins();
  pipe7.setupPins();
  Serial.println("Two-pipe game initialized.");
}

// --- Main Loop ---
void loop() {
  unsigned long now = millis();

  // 🔄 Update global blink state
  if (now - lastBlinkToggle >= BLINK_INTERVAL) {
    globalBlink = !globalBlink;
    lastBlinkToggle = now;
  }

  pipe1.update("Pipe 1", globalBlink);
  pipe2.update("Pipe 2", globalBlink);
  pipe3.update("Pipe 3", globalBlink);
  pipe5.update("Pipe 5", globalBlink);
  pipe6.update("Pipe 6", globalBlink);
  pipe7.update("Pipe 7", globalBlink);
}
