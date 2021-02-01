/*
  HCSR04 - Library for arduino, for HC-SR04 ultrasonic distance sensor.
  Created by Martin Sosic, June 11, 2016.
*/

#include "Arduino.h"
#include "HCSR04.h"

extern void log_printf(PGM_P fmt_P, ...);

UltraSonicDistanceSensor::UltraSonicDistanceSensor(
        int triggerPin, int echoPin) {
    this->triggerPin = triggerPin;
    this->echoPin = echoPin;
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

double UltraSonicDistanceSensor::measureDistanceCm() {
    //Using the approximate formula 19.307°C results in roughly 343m/s which is the commonly used value for air.
    return measureDistanceCm(19.307);
}

double UltraSonicDistanceSensor::measureDistanceCm(float temperature) {
    // Make sure that trigger pin is LOW.
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    // Hold trigger for 10 microseconds, which is signal for sensor to measure distance.
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    // Measure the length of echo signal, which is equal to the time needed for sound to go there and back.
    double speedOfSoundInCmPerMs = 0.03313 + 0.0000606 * temperature; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
    log_printf("speedOfSoundInCmPerMs %d", speedOfSoundInCmPerMs);
    log_printf("timeoutMs %d", 100 * 2 / speedOfSoundInCmPerMs);
    unsigned long durationMicroSec = pulseIn(echoPin, HIGH, 100 * 2 / speedOfSoundInCmPerMs);
    log_printf("durationMicroSec %d", durationMicroSec);
    if(durationMicroSec == 0){
        return -1.0;
    }

    double distanceCm = durationMicroSec / 2.0 * speedOfSoundInCmPerMs;
    if (distanceCm == 0 || distanceCm > 100) {
        return -1.0 ;
    } else {
        return distanceCm;
    }
}