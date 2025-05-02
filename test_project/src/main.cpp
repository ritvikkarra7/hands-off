#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"

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
#define echo2  GPIO_NUM_34

void setup()
{

  pinMode(echo1, INPUT); 
  pinMode(echo2, INPUT);
  pinMode(trig1, OUTPUT); 
  pinMode(trig2, OUTPUT); 

  Serial.begin(115200);

  Serial.println("Starting up");

  SPIFFS.begin();

  Serial.println("Created sample source"); 

  sampleSource = new WaveFormGenerator(40000, 2000, 0.01);

  // sampleSource = new WAVFileReader("/sample.wav");

  // Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);

}

float getDistance(int trig, int echo) {
  // distance in mm from sensor, with smoothing and frequency mapping

  static float smoothedDist = 0;  // persists between calls
  const float alpha = 0.2;        // smoothing factor
  const int dt = 10;

  digitalWrite(trig, LOW); 
  delayMicroseconds(dt); 
  digitalWrite(trig, HIGH); 
  delayMicroseconds(dt); 
  digitalWrite(trig, LOW); 

  int ping = pulseIn(echo, HIGH); 

  int rawDist = abs((347800 * ping * pow(10, -6)) / 2.0);

  // Apply exponential moving average
  if (smoothedDist == 0) smoothedDist = rawDist; // initialize on first call
  smoothedDist = alpha * rawDist + (1 - alpha) * smoothedDist;

  return 4509.7 * exp(-0.00619 * smoothedDist);
}


void loop()
{
  
  float frequency = getDistance(trig1, echo1); 
  float dist2 = getDistance(trig2, echo2); 

  Serial.printf("Pitch (distance 1): %f\n", frequency); 
  Serial.printf("Volume (distance 2): %f\n", dist2); 

  delay(10); 

  sampleSource->setWaveType(SINE); 
  sampleSource->setFrequency(frequency);
  sampleSource->setMagnitude(0.05); 


}