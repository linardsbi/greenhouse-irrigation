#include <Arduino.h>

#include "Plants.h"
#include "../Define.h"

unsigned Plants::getSoilMoisture() {
    pinMode(MOISTURE_POWER_PIN, OUTPUT);
    
    digitalWrite(MOISTURE_POWER_PIN, HIGH);
    delay(10);

    unsigned val = analogRead(A1);
    digitalWrite(MOISTURE_POWER_PIN, LOW);

    if (val > 910 || val < 550) return 100;

    val = map(val, 550, 910, 0, 99);
    
    return val;
}

const bool Plants::openValve() {
    Serial.println("open valve");
    digitalWrite(PIPE_RELAY_PIN, HIGH);
}
const bool Plants::closeValve() {
    Serial.println("close valve");
    digitalWrite(PIPE_RELAY_PIN, LOW);
}
const bool Plants::valveState() {
    return digitalRead(PIPE_RELAY_PIN);
}