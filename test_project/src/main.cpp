#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"
#include "Server.h"
#include <NewPing.h>

// TODO: 
// 1) Remove distance2freq and frequencyTask -- convert seconds directly to frequency 
// 2) Use better moving average for distance 
//    a) this could be done using NewPing::ping_median(). However, RTOS PRIORITY is VERY important for this to work. 
//    b) else use a circular buffer to store the last N samples and average them
// 3) Add the use of scales from website to select a scale to play in 

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_14,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *sampleSource;

#define trig_freq GPIO_NUM_32
#define trig2 GPIO_NUM_33
#define echo_freq GPIO_NUM_39
#define echo2 GPIO_NUM_34

TaskHandle_t frequencyTaskHandle = NULL;

// Function to calculate distance using ultrasonic sensor
void frequencyTask(void *params) {

  pinMode(trig_freq, OUTPUT);
  pinMode(echo_freq, INPUT);

  while (true) {
    digitalWrite(trig_freq, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_freq, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_freq, LOW);

    int duration = pulseIn(echo_freq, HIGH);

    static float smoothedDuration = 0;
    const float alpha = 0.2;

    if (smoothedDuration == 0) smoothedDuration = duration;
    smoothedDuration = alpha * duration + (1 - alpha) * smoothedDuration;

    float frequency = 4509.7 * exp(-0.001075 * smoothedDuration);

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