// Piezo sensor input pin.
const int SENSOR_PIN = A0;
const int INDICATOR_LED = LED_BUILTIN;
const int ON  = HIGH;
const int OFF = LOW;

const int BLUE_PIN  = 2;
const int GREEN_PIN = 3;
const int RED_PIN   = 4;

const int TOTAL_COLORS = 3;
const int LEDS[TOTAL_COLORS] = {RED_PIN, GREEN_PIN, BLUE_PIN};

// Input must be above this to be registered as a valid knock.
const int THRESHOLD = 10;

// Last read knock value.
int sensorValue = 0;

// Knock readings.
const int MAX_KNOCKS = 15;
int knockReadings[MAX_KNOCKS];

// Secret knock.
const int SECRET_KNOCK[MAX_KNOCKS] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Debounce.
const int DEBOUNCE = 150;

// Stop listening if there is no knock for END_OF_KNOCK milliseconds.
const int TIME_TO_WAIT = 1500;

// Error sway.
const int ERROR_ALLOWED = 25;

void setup()
{
  Serial.begin(9600);
  pinMode(INDICATOR_LED, OUTPUT);
}

void loop()
{
  sensorValue = analogRead(SENSOR_PIN);

  turnOnLed(BLUE_PIN);
  if (isKnock(sensorValue)) {
    // Start listening for a knocks.
    startListening();
  }
}

// Is knock actually knock enough?
bool isKnock(int input)
{
  return input > THRESHOLD;
}

void startListening()
{
  // Notify about listening.
  Serial.println("Got the first knock...");

  // Reset readings.
  int i = 0;
  for (i = 0; i < MAX_KNOCKS; i++)
    knockReadings[i] = 0;

  int knockCounter = 0;
  int lastKnockTime = millis();
  int now = millis();

  delay(DEBOUNCE);

  // Keep reading knocks.
  do {
    sensorValue = analogRead(SENSOR_PIN);
    if (isKnock(sensorValue)) {
      now = millis();
      knockReadings[knockCounter++] = now - lastKnockTime;
      lastKnockTime = now;

      digitalWrite(INDICATOR_LED, ON);
      delay(DEBOUNCE);
      digitalWrite(INDICATOR_LED, OFF);
    }

    now = millis();
  } while ((now - lastKnockTime) < TIME_TO_WAIT);

  Serial.println("Finished listening for knocks, now verifying...");

  for (i = 0; i < MAX_KNOCKS; i++) {
    Serial.print(knockReadings[i]);
    Serial.print(", ");
  }
  Serial.println("");

  if (validateKnock()) {
    Serial.println("Knock matches, welcome!");
    turnOnLed(GREEN_PIN);
  } else {
    Serial.println("Knock did not match, go away!");
    turnOnLed(RED_PIN);
  }

  delay(2000);
}

bool validateKnock()
{
  int i, knockCount = 0, secretKnockCount = 0;
  int maxKnockInterval = 0;

  // Count knocks.
  for (i = 0; i < MAX_KNOCKS; i++) {
    if (knockReadings[i] > 0)
      knockCount++;

    if (SECRET_KNOCK[i] > 0)
      secretKnockCount++;

    if (knockReadings[i] > maxKnockInterval)
      maxKnockInterval = knockReadings[i];
  }

  // Do knock counts match?
  if (knockCount != secretKnockCount)
    return false;

  int timeError, totalTimeError = 0;
  for (i = 0; i < MAX_KNOCKS; i++) {
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);

//    Serial.println(knockReadings[i]);

    timeError = abs(knockReadings[i] - SECRET_KNOCK[i]);
    if (timeError > ERROR_ALLOWED)
      return false;

    totalTimeError += timeError;
  }

  if (totalTimeError / secretKnockCount > ERROR_ALLOWED) {
    return false;
  }

  return true;
}

void turnOnLed(int color)
{
  for (int i = 0; i < TOTAL_COLORS; i++) {
    digitalWrite(LEDS[i], LOW);
  }

  digitalWrite(color, HIGH);
}


