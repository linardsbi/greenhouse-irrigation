#include <Arduino.h>
#include <math.h>

#include "Watering.h"
#include "Define.h"
#include "DHT.h"

DHT dht(DHT_PIN, DHT11);

unsigned Logging::getAmperage() {
    unsigned mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module
    unsigned ACSoffset = 2500;
    unsigned voltage = 0;

    return ((voltage - ACSoffset) / mVperAmp);
}

float Logging::getTemperature() {
    return dht.readTemperature();
}

float Logging::getHumidity() {
    return dht.readHumidity();
}

void Watering::runTests() {
    Serial.println("Running tests");

    Serial.print("Getting tank fill amount: ");
    Serial.println(Tank::getAmountFilled());

    Serial.print("Getting air temperature and moisture: ");
    Serial.println(Logging::getTemperature());
    Serial.println(Logging::getHumidity());

    Serial.print("Getting plant moisture level: ");
    Serial.println(Plants::getSoilMoisture());

    Serial.print("Getting amperage: ");
    Serial.println(Logging::getAmperage());

    Serial.print("Pinging address result: ");
    Serial.println(Lora::pingModule());

    delay(2000);
    runTests();
}

void Watering::start(const unsigned baudRate = 9600) {
    RET_STATUS STATUS = RET_SUCCESS;

    Serial.begin(baudRate);
    Serial.println("starting");
    
    #ifdef ULTRASONIC_SENSOR
        pinMode(PUMP_RELAY_PIN, OUTPUT);
        digitalWrite(PUMP_RELAY_PIN, LOW);
    #endif
    
    pinMode(PIPE_RELAY_PIN, OUTPUT);
    digitalWrite(PIPE_RELAY_PIN, LOW);

    dht.begin();

    STATUS = Lora::startLoraModule();

    if(STATUS == RET_SUCCESS) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("started");
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }

    #ifdef DEBUG
        // runTests();
    #endif
}

std::vector<std::vector<unsigned>> Watering::gatherData(const uint8_t *sensors, const uint8_t arrLength) {
    std::vector<std::vector<unsigned>> readings;  
    // todo: fix, it references this variable. 
    // unsigned pair[2] = {S_NONE, 0};
    std::vector<unsigned> pair = {S_NONE, 0};
    if (arrLength > 1) {
        for (unsigned i = 0; i < arrLength; i++) {
            
            switch (sensors[i])
            {
                case S_MOISTURE:
                    pair[SENS_TYPE] = S_MOISTURE;
                    pair[SENS_VALUE] = Plants::getSoilMoisture();
                    break;
                case S_TANK_FILL:
                    pair[SENS_TYPE] = S_TANK_FILL;
                    pair[SENS_VALUE] = Tank::getAmountFilled();
                    break;
                case S_PUMP_STATE:
                    pair[SENS_TYPE] = S_PUMP_STATE;
                    pair[SENS_VALUE] = Tank::pumpState();
                    break;
                case S_HOSE_STATE:
                    pair[SENS_TYPE] = S_HOSE_STATE;
                    pair[SENS_VALUE] = Plants::valveState();
                    break;
                case S_HUMIDITY:
                    pair[SENS_TYPE] = S_HUMIDITY;
                    pair[SENS_VALUE] = (unsigned)Logging::getHumidity();
                    break;
                case S_TEMPERATURE:
                    pair[SENS_TYPE] = S_TEMPERATURE;
                    pair[SENS_VALUE] = Logging::getTemperature() + 100;
                    break;  
                case S_CURRENT:
                    pair[SENS_TYPE] = S_CURRENT;
                    pair[SENS_VALUE] = Logging::getAmperage();
                    break;    
                default:
                    #ifdef DEBUG
                        Serial.print("Invalid parameter: ");
                        Serial.println(sensors[i]);
                    #endif

                    break;
            }
            // Serial.println(sensors[i]);
            // Serial.print("sensor: ");
            // Serial.println(pair[SENS_TYPE]);
            // Serial.print("val: ");
            // Serial.println(pair[SENS_VALUE]);
            readings.push_back(pair);
        }
    }

    return readings;
}

// S_MOISTURE, S_TANK_FILL, S_PUMP_STATE, S_HOSE_STATE, S_HUMIDITY, S_TEMPERATURE, S_CURRENT
RET_STATUS Watering::sendData(const uint8_t *sensors = defaultSensors, const uint8_t arrLength) {
    RET_STATUS STATUS = RET_SUCCESS;

    std::vector<std::vector<unsigned>> readings = gatherData(sensors, arrLength);

    STATUS = Lora::createAndSendBuffer(readings);
    
    return STATUS;
}