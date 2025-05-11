#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"
#include "Server.h"

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_14,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *sampleSource;

#define trig1 GPIO_NUM_32
#define trig2 GPIO_NUM_33
#define echo1 GPIO_NUM_39
#define echo2 GPIO_NUM_34

TaskHandle_t sensor1TaskHandle = NULL;
TaskHandle_t sensor2TaskHandle = NULL;
TaskHandle_t frequencyTaskHandle = NULL;

volatile float distance1 = 0; // Distance from sensor 1
volatile float distance2 = 0; // Distance from sensor 2

// Function to calculate distance using ultrasonic sensor
void ultrasonicTask(void *params) {
  // Extract trigger and echo pins from the task parameters
  int *pins = (int *)params;
  int trigPin = pins[0];
  int echoPin = pins[1];
  float *distance = (float *)pins[2];

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  while (true) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    int duration = pulseIn(echoPin, HIGH);
    *distance = abs((347800 * duration * pow(10, -6)) / 2.0);
    vTaskDelay(50 / portTICK_PERIOD_MS); 
  }
}

// Function to map distance to frequency
float dist2freq(float dist) {
  static float smoothedDist = 0;  // persists between calls
  const float alpha = 0.2;        // smoothing factor

  // Apply exponential moving average
  if (smoothedDist == 0) smoothedDist = dist; // initialize on first call
  smoothedDist = alpha * dist + (1 - alpha) * smoothedDist;

  return 4509.7 * exp(-0.00619 * smoothedDist);
}

// Task to convert distance to frequency and update the waveform generator
void frequencyTask(void *params) {
  while (true) {
    float frequency = dist2freq(distance1); // Convert distance1 to frequency
    sampleSource->setFrequency(frequency); // Update the waveform generator
    sampleSource->setMagnitude(0.05);      // Set a fixed magnitude

    // Broadcast the frequency to WebSocket clients
    broadcastFrequency(frequency);

    vTaskDelay(20 / portTICK_PERIOD_MS); // Update frequency every 20ms
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  SPIFFS.begin();

  startWebServices();

  sampleSource = new WaveFormGenerator(40000, 2000, 0.01);

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);

  // Create tasks for each ultrasonic sensor
  static int sensor1Params[] = {trig1, echo1, (int)&distance1};
  static int sensor2Params[] = {trig2, echo2, (int)&distance2};

  xTaskCreatePinnedToCore(
    ultrasonicTask,
    "Sensor1Task",
    1024,
    sensor1Params,
    2,
    &sensor1TaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    ultrasonicTask,
    "Sensor2Task",
    1024,
    sensor2Params,
    2,
    &sensor2TaskHandle,
    1
  );

  // Create a task to handle frequency updates
  xTaskCreatePinnedToCore(
    frequencyTask,
    "FrequencyTask",
    2048,
    NULL,
    2,
    &frequencyTaskHandle,
    1
  );

  vTaskDelete(NULL);
}

void loop() {
  // Empty loop as all tasks are handled by FreeRTOS
}