#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"
#include "Server.h"
#include "UltrasonicSensor.h"

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

TaskHandle_t frequencyTaskHandle = NULL;

UltrasonicSensor freqSensor(trig_freq, echo_freq, 2.0, 100.0);
UltrasonicSensor volSensor(trig_vol, echo_vol, 2.0, 100.0);

// Function to calculate distance using ultrasonic sensor
void frequencyTask(void *params) {

  while (true) {

    float duration = freqSensor.readDuration();
    float frequency = 4509.7 * exp(-0.001075 * duration);

    if (frequency < 20) 
    {
      sampleSource->setFrequency(0);
      sampleSource->setMagnitude(0);
      broadcastFrequency(0);
    }
    else {
      sampleSource->setFrequency(frequency);
      sampleSource->setMagnitude(0.05);
      broadcastFrequency(frequency);
    }

    vTaskDelay(20 / portTICK_PERIOD_MS); 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  freqSensor.begin();

  SPIFFS.begin();

  startWebServices();

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

  vTaskDelete(NULL);
}

void loop() {
  // Empty loop as all tasks are handled by FreeRTOS
}