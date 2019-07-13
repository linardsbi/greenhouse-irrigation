 #include "./src/Watering.h"
 #include "./src/Define.h"

// S_MOISTURE, S_TANK_FILL, S_PUMP_STATE, S_HOSE_STATE, S_HUMIDITY, S_TEMPERATURE, S_CURRENT
const uint8_t sensorsToSendFrom[] = {S_MOISTURE, S_TANK_FILL, S_PUMP_STATE, S_HOSE_STATE, S_HUMIDITY, S_TEMPERATURE, S_CURRENT};

void setup() {
  Watering::start(9600);

  runMain();
}

/**
 * TODO:
 * decide how often should the data be sent
 * decide how often main loop should be run
 * hash check on target ping
 */
void runMain() {
  RET_STATUS status = RET_SUCCESS;
  // while (Tank::getAmountFilled() < REFILL_POINT) {
    
    while (Plants::getSoilMoisture() < MOISTURE_POINT) {
      Serial.println("soil needs water");
      while (Tank::getAmountFilled() < REFILL_POINT) {
        Tank::pumpOn();
        Serial.println("tank not filled");
        delay(10000);
      }

      Tank::pumpOff();

      Plants::openValve();
      delay(5000);
    }
    
    Plants::closeValve();
  // }

  const uint8_t length = sizeof(sensorsToSendFrom) / sizeof(sensorsToSendFrom[0]);

  status = Watering::sendData(sensorsToSendFrom, length);

  Serial.print("status: ");
  Serial.println(status);

  Serial.println("finished loop");
  delay(1000);
  runMain();
}

void loop() {}