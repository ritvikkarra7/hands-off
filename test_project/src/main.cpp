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

UltrasonicSensor freqSensor(trig_freq, echo_freq, 2.0, 100.0);
UltrasonicSensor volSensor(trig_vol, echo_vol, 2.0, 100.0);

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
      sampleSource->setMagnitude(0.05);
      broadcastFrequency(frequency);
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void potTask(void *params) {
  while (true) {

    int potValue = analogRead(pot);
    byte gain = map(potValue, 0, 4095, 0, 255);

    digipot.setWiperBoth(gain); 

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void volumeTask(void *params) {
  while (true) {
    float duration = volSensor.readDuration();
    double volume = map(duration, 0, 500, 255, 0); // Map duration to volume (0 to 1)
    volume = constrain(volume, 0, 255); // Ensure volume is within bounds
    digipot.setWiperBoth((byte)volume); // Set the digital potentiometer wiper
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  freqSensor.begin();
  volSensor.begin();
  digipot.setWiper(200); // Initialize digital potentiometer to 0

  SPIFFS.begin();

  startWebServices();

  pinMode(SEL, OUTPUT); // GPIO for the multiplexer control
  digitalWrite(SEL, LOW); // mux is low for digital mode by default
  sampleSource = new WaveFormGenerator(40000, 2000, 0.01);

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);

  // Create a task to handle frequency updates
  xTaskCreatePinnedToCore(
    frequencyTask,
    "FrequencyTask",
    4096,
    NULL,
    2,
    &frequencyTaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    potTask, 
    "PotentiometerTask",
    4096, 
    NULL, 
    2, 
    &potTaskHandle, 
    1
  ); 

  xTaskCreatePinnedToCore(
    volumeTask, 
    "VolumeTask",
    4096, 
    NULL, 
    2, 
    &volumeTaskHandle, 
    1
  );

  vTaskDelete(NULL);
}

void loop() {
  // Empty loop as all tasks are handled by FreeRTOS
}