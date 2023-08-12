#include "Joystick.h"

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_MULTI_AXIS, 3, 0,
  true, false, false, false, false, false,
  false, false, false, false, false);

#define PIN_A 4
#define PIN_B 2
#define PIN_LED 8  // provides at least some info about current mode (setup/loop)
#define PIN_CENTER A0

#define INTERVAL_MS 20 // 20

int TICKS = 0;
int RANGE_MIN = 0;
int RANGE_MAX = 0;

bool isCenterValue(int value) {
    return value < 10; // <= 8 also good, but edgy
}

int getTicksPerInterval() {
    int ticks_per_interval = 0;
    
    bool oldA = digitalRead(PIN_A);
    bool oldB = digitalRead(PIN_B);
    
    unsigned long time_now = millis();

    while (millis() < (time_now + INTERVAL_MS)) {
        bool A = digitalRead(PIN_A);
        bool B = digitalRead(PIN_B);
        int C = analogRead(PIN_CENTER);
        //LogSensorStatus(A, B, C);
        bool isCenter = isCenterValue(C);

        if ((oldA != A)) {
            if (isCenter) {
                TICKS = 0;
                return 0;
            }
          
            bool CW = (A == B); // If I and Q are the same, CW, otherwise CCW
            if (CW) {
                ++ticks_per_interval;
            }
            else {
                --ticks_per_interval;
            }
        }
        else if ((oldB != B)) {
            if (isCenter) {
                TICKS = 0;
                return 0;
            }
            bool CW = (A != B); // If I and Q are not the same, CW, otherwise CCW
            if (CW) {
                ++ticks_per_interval;
            }
            else {
                --ticks_per_interval;
            }
        }
        oldA = A;
        oldB = B;
    }

    return ticks_per_interval;
}


void findFirstCWCenterTick() {
    Serial.println("1. Please, move the wheel clock-wise from 10 o'clock to 4 o'clock");
    
    bool center_found = false;

    while (!center_found) {
        bool oldA = digitalRead(PIN_A);
        bool oldB = digitalRead(PIN_B);
        unsigned long time_now = millis();

        while (millis() < (time_now + INTERVAL_MS)) {
            bool A = digitalRead(PIN_A);
            bool B = digitalRead(PIN_B);
            
            int C = analogRead(PIN_CENTER);
            //LogSensorStatus(A, B, C);
            bool isCenter = isCenterValue(C);

            if ((oldA != A)) {
                bool CW = (A == B); // If I and Q are the same, CW, otherwise CCW
                if (CW) {
                    if (isCenter) {
                        center_found = true;
                        break;
                    }
                }
            }
            else if ((oldB != B)) {
                bool CW = (A != B); // If I and Q are not the same, CW, otherwise CCW
                if (CW) {
                    if (isCenter) {
                        center_found = true;
                        break;
                    }
                }
            }
            oldA = A;
            oldB = B;
        }
    }
}

int getCWTicksPerFullRevolution() {
    Serial.println("2. Continue to move the wheel CW to full right, then turn wheel CCW");

    int ticks_per_full_revolution = 0;
    bool full_revolution_found = false;
    
    while (!full_revolution_found) {        
        bool oldA = digitalRead(PIN_A);
        bool oldB = digitalRead(PIN_B);
        unsigned long time_now = millis();
        
        int ticks_per_interval = 0;

        while (millis() < (time_now + INTERVAL_MS)) {
            bool A = digitalRead(PIN_A);
            bool B = digitalRead(PIN_B);

            if ((oldA != A)) {
                bool CW = (A == B); // If I and Q are the same, CW, otherwise CCW
                if (CW) {
                    ++ticks_per_interval;
                }
                else {
                    full_revolution_found = true;
                    break;
                }
            }
            else if ((oldB != B)) {
                bool CW = (A != B); // If I and Q are not the same, CW, otherwise CCW
                if (CW) {
                    ++ticks_per_interval;
                }
                else {
                    full_revolution_found = true;
                    break;
                }
            }
            oldA = A;
            oldB = B;
        }

        ticks_per_full_revolution = ticks_per_full_revolution + ticks_per_interval;
    }
    Serial.println("CCW detected, stopping");
    return ticks_per_full_revolution;
}


int getCCWTicksToFirstCenterTick() {
    Serial.println("3. Move the wheel CCW to 12 o'clock (center) to finish calibration");

    int ticks_to_center = 0;
    bool center_found = false;
    
    while (!center_found) {
        bool oldA = digitalRead(PIN_A);
        bool oldB = digitalRead(PIN_B);
        unsigned long time_now = millis();

        int ticks_per_interval = 0;

        while (millis() < (time_now + INTERVAL_MS)) {
            bool A = digitalRead(PIN_A);
            bool B = digitalRead(PIN_B);
            int C = analogRead(PIN_CENTER);
            //LogSensorStatus(A, B, C);
            bool isCenter = isCenterValue(C);
            if (isCenter) {
                center_found = true;
                break;
            }

            if ((oldA != A)) {
                bool CW = (A == B); // If I and Q are the same, CW, otherwise CCW
                if (!CW) {
                    ++ticks_per_interval;
                }
            }
            else if ((oldB != B)) {
                bool CW = (A != B); // If I and Q are not the same, CW, otherwise CCW
                if (!CW) {
                    ++ticks_per_interval;
                }
            }
            oldA = A;
            oldB = B;
        }

        ticks_to_center = ticks_to_center + ticks_per_interval;
    }

    return ticks_to_center;
}

void LogSensorStatus(bool a, bool b, int c) {
    return;
    Serial.print("Sensor status: ");
    Serial.print(a);
    Serial.print(b);
    Serial.print(" is center: ");
    Serial.print(c);
    Serial.print("\n");
}

void setSteering()
{
    //Serial.println("Ticks actual: ");
    //Serial.print(ticks);
    //Serial.print(", registered: ");
    /*
    if (TICKS > RANGE_MAX) {
        TICKS = RANGE_MAX;
    }
    else if (TICKS < RANGE_MIN) {
        TICKS = RANGE_MIN;
    }
    */
    //Serial.print(value);
    //Serial.print("\n");
    
    Joystick.setXAxis(TICKS);
}

void setup() {
    // put your setup code here, to run once:
    Serial.println("On setup()");

    pinMode(PIN_A, INPUT);
    pinMode(PIN_B, INPUT);
    pinMode(PIN_CENTER, INPUT);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);

    Serial.print("Finding the center!\n");
    findFirstCWCenterTick();
    int center = 0;
    int ticksToFullRevolution = getCWTicksPerFullRevolution();
    
    Serial.print("Max range:");
    Serial.print(RANGE_MAX);
    Serial.print("\n");
    
    int ticksToCenter = getCCWTicksToFirstCenterTick();
    
    Serial.print("Distance from center to full-right:");
    Serial.print(ticksToFullRevolution);
    Serial.print("\n");
    Serial.print("Distance from full-right to center:");
    Serial.print(ticksToCenter);
    Serial.print("\n");
    //center = (ticksToFullRevolution - ticksToCenter) / 4;
    //Serial.print("New center:");
    //Serial.print(center);
    //Serial.print("\n");
    
    TICKS = 0;
    RANGE_MAX = ticksToCenter;
    RANGE_MIN = -RANGE_MAX;
    digitalWrite(PIN_LED, LOW);

    Joystick.setXAxisRange(-RANGE_MAX, RANGE_MAX);
    Joystick.begin();
}

void loop() {    
    TICKS = TICKS + getTicksPerInterval();
    setSteering();
}

