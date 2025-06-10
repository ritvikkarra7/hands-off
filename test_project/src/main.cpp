#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"
#include "Server.h"
#include "UltrasonicSensor.h"
#include "CAT5171.h"
#include "utilities.h"

// TODO: 
// 1) Handle mode switching from website
// 2) Add the use of scales from website to select a scale to play in 

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_14,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *sampleSource;

#define trig_freq GPIO_NUM_32
#define trig_vol GPIO_NUM_33
#define echo_freq GPIO_NUM_39
#define echo_vol GPIO_NUM_34
#define pot GPIO_NUM_35

TaskHandle_t frequencyTaskHandle = NULL;
TaskHandle_t potTaskHandle = NULL;
TaskHandle_t volumeTaskHandle = NULL;
CAT5171 digipot; // Digital Potentiometer for volume control

UltrasonicSensor freqSensor(trig_freq, echo_freq, 1.0, 100.0);
UltrasonicSensor volSensor(trig_vol, echo_vol, 1.0, 100.0);

double magnitude = 0.05; 

// Function to calculate distance using ultrasonic sensor
void frequencyTask(void *params) {
  while (true) {
    float duration = freqSensor.readDuration();
    float frequency = sampleSource->getNoteFromDuration(duration);

    if (frequency < 20 || isnan(frequency)) {
      sampleSource->setFrequency(0);
      sampleSource->setMagnitude(0);
      broadcastFrequency(0);
    } else {
      sampleSource->setFrequency(frequency);
      sampleSource->setMagnitude(magnitude);
      broadcastFrequency(frequency);
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void potTask(void *params) {
  while (true) {

    Serial.println("Potentiometer Task Running");
    if (digitalRead(SEL) == HIGH){

      int potValue = analogRead(pot);
      byte gain = map(potValue, 0, 4095, 0, 255);
      Serial.println("Potentiometer Value: " + String(potValue) + ", Gain: " + String(gain));
      digipot.setWiperBoth(gain); 
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);

  }
}

void volumeTask(void *params) {
  while (true) {

    Serial.println("Volume Task Running");
    if (digitalRead(SEL) == LOW) {

      float duration = volSensor.readDuration();
      magnitude = 0.3 * exp(-0.00285 * duration); 
      Serial.println("Volume Duration: " + String(duration) + " ms, Magnitude: " + String(magnitude));
      if (magnitude > 0.3 || magnitude < 0.0001) {
        sampleSource->setMagnitude(0); // Cap volume at 0.3
      } 
      sampleSource->setMagnitude(magnitude); // Set magnitude based on volume

    }

    vTaskDelay(20 / portTICK_PERIOD_MS);

  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  freqSensor.begin();
  volSensor.begin();
  digipot.setWiperBoth(200); // Initialize digital potentiometer to 0

  SPIFFS.begin();

  startWebServices();

  pinMode(SEL, OUTPUT); // GPIO for the multiplexer control
  digitalWrite(SEL, LOW); // mux is low for digital mode by default
  sampleSource = new WaveFormGenerator(40000, 2000, 0.05);

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);

  // Create a task to handle frequency updates
  xTaskCreate(
    frequencyTask,
    "FrequencyTask",
    4096,
    NULL,
    2,
    &frequencyTaskHandle
  );

  xTaskCreate(
    potTask, 
    "PotentiometerTask",
    4096, 
    NULL, 
    2, 
    &potTaskHandle
  ); 

  xTaskCreate(
    volumeTask, 
    "VolumeTask",
    8192, 
    NULL, 
    2, 
    &volumeTaskHandle
  );

  vTaskDelete(NULL);
}

void loop() {
  // Empty loop as all tasks are handled by FreeRTOS
}