// Sunsong - a light triggered music box crank
#include <Servo.h>

#define _noDEBUG

// Dexfine which pins each of our sensors and actuators are connected to
#define PIN_SERVO 9
#define PIN_CDS A0

#define LIGHT_CHANGE_THRESHOLD 150

#define PLAY_DURATION_SECS 19
const int PLAY_DURATION_MS = PLAY_DURATION_SECS * 1000;

#define SENSOR_FREQUENCY_SECS 1
const int SENSOR_FREQUENCY_MS = SENSOR_FREQUENCY_SECS * 1000;
#define MAX_SENSOR_READING 1024

bool wakeup_from_sensors;
unsigned long sleep_since, last_sensor_activity_at, playing_since, next_sensor_at;

int  prev_light_level, light_delta;

#define LIGHT_SAMPLES 5

Servo crank;

// Define these based on your servos and controller, the values to cause your servos
// to spin in opposite directions at approx the same speed.
#define CCW 30
#define CW 150

#define CCW_SLOW 80
#define CW_SLOW 100 

#define CW_STOP 90
#define CCW_STOP 90

const int SERVO_STOP = CW_STOP;
const int SERVO_FWD = CW;

// How long to spin while callibrating the sensor pair
#define CALLIBRATION_DUR_MS 1000

void stop() {
  #ifdef _DEBUG
  Serial.println("stop");
  #endif
  crank.write(SERVO_STOP);
}

void setup() {
  #ifdef _DEBUG
  Serial.begin(9600);
  Serial.println("setup");
  #endif
  crank.attach(PIN_SERVO);
  stop();
  sleep_since = 0L;
  last_sensor_activity_at = 0L;
  playing_since = 0L;
  next_sensor_at = 0L;
  prev_light_level = 0;
}


int smooth(int array[], int len) {
  /**
    Return the average of the array without the highest and lowest values.
  **/
  int low = MAX_SENSOR_READING;
  int high = -1;
  int total = 0;
  for (int s = 0; s < len; s++) {
    total += array[s];
    low = min(array[s], low);
    high = max(array[s], high);
  }
  total -= low;
  total -= high;
  return total / (len - 2);
}

void readSensors() {
  if (next_sensor_at > millis()) {
    return;
  }
  next_sensor_at = millis() + SENSOR_FREQUENCY_MS;
  
  wakeup_from_sensors = false;
  int l = getLightLevel();
  #ifdef _DEBUG
  Serial.print("light level: ");
  Serial.print(prev_light_level);
  Serial.print(" => ");
  Serial.println(l);
  #endif

  light_delta = l - prev_light_level;
  prev_light_level = l;
  if ((light_delta > LIGHT_CHANGE_THRESHOLD)) {
    last_sensor_activity_at = millis();
    wakeup_from_sensors = true;
    #ifdef _DEBUG
    Serial.println("sensor input");
    #endif
  }
}

int getLightLevel() {
  // Return the median reading from the light sensor
  int samples[LIGHT_SAMPLES];
  for (int sample = 0; sample < LIGHT_SAMPLES; sample++) {
    samples[sample] = analogRead(PIN_CDS);
  }
  return smooth(samples, LIGHT_SAMPLES);
}

void play() {
  #ifdef _DEBUG
  Serial.println("play");
  #endif
  sleep_since = 0L;
  crank.write(SERVO_FWD);
}

void expirePlaying() {
  if (playing_since && ((playing_since + PLAY_DURATION_MS) < millis())) {
    playing_since = 0L;
    sleep_since = millis();
    stop();
  }
}
void loop() {
  readSensors();
  if (wakeup_from_sensors && !playing_since) {
    playing_since = millis();
  }
  if (playing_since) {
    play();
  }
  expirePlaying();
}

