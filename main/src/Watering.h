#include <ArduinoSTL.h>

#include "./LoRa/Lora.h"
#include "./Plants/Plants.h"
#include "./Tank/Tank.h"

#ifndef WATERING_H
#define WATERING_H

enum SensorType {
    S_NONE = 0,
    S_MOISTURE,
    S_TANK_FILL,
    S_PUMP_STATE,
    S_HOSE_STATE,
    S_HUMIDITY,
    S_TEMPERATURE,
    S_CURRENT,
};

enum {
    SENS_TYPE = 0,
    SENS_VALUE
};

const uint8_t defaultSensors[1] = {S_NONE};

class Logging {
    public:
        static unsigned getAmperage();
        static float getTemperature();
        static float getHumidity();
    private:
        
};

class Watering {
    public:
        static void start(const unsigned baudRate = 9600);
        static RET_STATUS sendData(const uint8_t *sensors = defaultSensors, const uint8_t arrLength = 1);
    private:
        static void runTests();
        static std::vector<std::vector<unsigned>> gatherData(const uint8_t *sensors, const uint8_t arrLength);
};

#endif