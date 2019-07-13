#include <Arduino.h>
#include "NewPing.h"

class Tank {
    public:
        static unsigned getAmountFilled();
        static const bool pumpOff();
        static const bool pumpOn();
        static const bool pumpState();
};
