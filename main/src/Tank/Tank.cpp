#include "Tank.h"
#include "../Define.h"

NewPing sonar(TRIG_PIN, ECHO_PIN, TANK_DEPTH);

unsigned Tank::getAmountFilled() {
    return ((TANK_DEPTH - sonar.ping_cm()) / (float)TANK_DEPTH) * 100;
}

const bool Tank::pumpOff() {
    Serial.println("pump off");
    digitalWrite(PUMP_RELAY_PIN, LOW);
}

const bool Tank::pumpOn() {
    Serial.println("pump on");
    digitalWrite(PUMP_RELAY_PIN, HIGH);
}

const bool Tank::pumpState() {
    return digitalRead(PUMP_RELAY_PIN);
}


