#include <Servo.h>

int photoPinLeft = A0; //green
int photoPinRight = A1; //blue
int photoPinTop = A2;  //orange
int photoPinBottom = A3; //brown


class Seeker {
    Servo servo;
    int val1;
    int val2;
    int tol;    //Adjustment so it doesn't constantly move
    int pos;
    int startPos;
    int endPos;
    int parkPos;
    int updateInterval;     //How often the position is updated in ms
    unsigned long lastUpdate;
    int servoRelayOff = 5;
    int servoRelayOn = 6;
    int servoRelayState = 11;
    int piLightLevel = 13;

public:
    Seeker(int startP, int endP, int parkP) {
        startPos = startP;
        endPos = endP;
        parkPos = parkP;
        lastUpdate = 0;
        updateInterval = 100;
        tol = 2;
        servoRelayOff = 5;
        servoRelayOn = 6;
        servoRelayState = 11;
        piLightLevel = 13;
        pinMode(servoRelayOff, OUTPUT);
        pinMode(servoRelayOn, OUTPUT);
        pinMode(servoRelayState, INPUT);
        pinMode(piLightLevel, OUTPUT);
    }

    void Attach(int pin) {
        servo.attach(pin);
    }

    void Detach() {
        servo.detach();
    }

    void relay_on() {
        digitalWrite(servoRelayOn, HIGH);
        delay(10);
        digitalWrite(servoRelayOn, LOW);
    }

    void relay_off() {
        digitalWrite(servoRelayOff, HIGH);
        delay(10);
        digitalWrite(servoRelayOff, LOW);
    }

    void park_pos() {
        if (digitalRead(servoRelayState) == HIGH) {
            pos = parkPos;
            servo.write(pos);
            delay(2000);
            relay_off();
        }
        digitalWrite(piLightLevel, LOW);
    }

    void hold_pos() {
        digitalWrite(piLightLevel, LOW);
    }

    void track_sun(int val1, int val2) {
        digitalWrite(piLightLevel, HIGH);
        if (digitalRead(servoRelayState) == LOW) {
            relay_on();
        }
        if ((abs(val1 - val2) <= tol) && (abs(val2 - val1) <= tol)) {
            pos = pos;   //Hold position when photo resistors are within the tolerance
        } else if (pos > endPos) {
            pos = endPos;
        } else if (pos < startPos) {
            pos = startPos;
        } else if (val1 > val2) {
            pos = --pos;
        } else if (val2 > val1) {
            pos = ++pos;
        }
        servo.write(pos);
        delay(50);
    }

    void update(int pin1, int pin2, String sunLevel) {

        val1 = analogRead(pin1);
        val2 = analogRead(pin2);

        if((millis() - lastUpdate) > updateInterval) {
            lastUpdate = millis();
            if (sunLevel == "low") {
                park_pos();
            } else if (sunLevel == "medium") {
                hold_pos();
            } else if (sunLevel == "high") {
                track_sun(val1, val2);
            }
        }
    }
};

class LightLevel {
    int totalSensorAvg;
    int avgPhotoLevel;
    int sensorAvg;
    int lightLevel;
    int lowLightCutoff;
    int highLightCutoff;

public:
    int sensor_avg() {
        totalSensorAvg = 0;
        for (int i = 0; i < 10; i++) {
            sensorAvg = (analogRead(photoPinLeft) + analogRead(photoPinRight) + analogRead(photoPinTop) + analogRead(photoPinBottom)) / 4;
            totalSensorAvg += sensorAvg;
            delay(100);
        }
        avgPhotoLevel = totalSensorAvg / 10;
        return avgPhotoLevel;
    }

    String get_sun_level() {

        lowLightCutoff = 600;
        highLightCutoff = 700;

        lightLevel = sensor_avg();
        //return "high"; //Remove comment for debug
        //Serial.println("lightlevel:" + String(lightLevel));
        if (lightLevel <= lowLightCutoff ) {
            return "low";   // very low light
        } else if (lightLevel > lowLightCutoff && lightLevel < highLightCutoff) {
            return "medium";   // low light
        } else if (lightLevel >= highLightCutoff) {
            return "high";   // lots of light
        }
    }

};

Seeker hSeeker(0, 180, 90);
Seeker vSeeker(50, 130, 90);

LightLevel level;

void setup() {
    //Serial.begin(9600);
    hSeeker.Attach(9);
    vSeeker.Attach(10);
    hSeeker.relay_on();
    hSeeker.park_pos();
    vSeeker.park_pos();
}

void loop() {
    String lightLevel = level.get_sun_level();
    vSeeker.update(photoPinTop, photoPinBottom, lightLevel);
    hSeeker.update(photoPinLeft, photoPinRight, lightLevel);
}
