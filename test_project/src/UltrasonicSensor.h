#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
private:
    int trigPin;
    int echoPin;

    // Kalman filter variables
    double R;       // Measurement noise covariance
    double Q;       // Process noise covariance
    double P;       // Estimate error covariance
    double U_hat;   // Filtered estimate
    const double H = 1.0;  // Measurement map
    bool initialized;

public:
    UltrasonicSensor(int trig, int echo, double processNoise = 10.0, double measurementNoise = 40.0)
        : trigPin(trig), echoPin(echo), Q(processNoise), R(measurementNoise), P(0.0), U_hat(0.0), initialized(false) {}

    void begin() {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    float readDuration() {
        // Trigger ultrasonic ping
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        // Read echo duration
        int duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout

        // Kalman filter update
        if (!initialized) {
            U_hat = duration;
            initialized = true;
        }

        double K = P * H / (H * P * H + R);              // Kalman gain
        U_hat += K * (duration - H * U_hat);            // Update estimate
        P = (1 - K * H) * P + Q;                         // Update error covariance

        // Serial.println("reading duration"); 
        // convert duration to distance in cm
        // float distance = U_hat * 0.034 / 2; // Speed of sound is 0.034 cm/us, divided by 2 for round trip
        // Serial.printf("Distance: %.2f cm\n", distance);

        return U_hat;
    }

    void resetFilter(double initial_estimate = 0.0) {
        U_hat = initial_estimate;
        P = 0.0;
        initialized = false;
    }
};

#endif
