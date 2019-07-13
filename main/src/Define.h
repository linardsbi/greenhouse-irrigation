#define DEBUG

enum LoraPins {
    M0_PIN = 7,
    M1_PIN = 8,
    AUX_PIN = 17, // A0
    SOFT_RX = 10,
    SOFT_TX = 11
};

#define DEVICE_ADDR_H 0x05
#define DEVICE_ADDR_L 0x02
#define DEVICE_ADDR_CHANNEL 0x00

#define ULTRASONIC_SENSOR
#define TRIG_PIN 3
#define ECHO_PIN 2

#define AMPMETER_SENSOR
#define AMP_PIN 18 // A1

#define PUMP_RELAY_PIN 4

#define DHT_SENSOR
#define DHT_PIN 5

#define PIPE_RELAY_PIN 6

#define MOISTURE_PIN 19 // A2
#define MOISTURE_POWER_PIN 9

#define TANK_DEPTH 80    // in CM
#define REFILL_POINT 80   // in %
#define MOISTURE_POINT 40 // in %