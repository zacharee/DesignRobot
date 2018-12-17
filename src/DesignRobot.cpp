#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include "WemosInit.h"

uint8_t MOTOR_1 = 4;
uint8_t MOTOR_2 = 14;

uint8_t LEFT = 12;
uint8_t FRONT = 13;
uint8_t RIGHT = 15;
uint8_t BACK = 16;

int MOTOR_1_STOP = 92;
int MOTOR_2_STOP = 92;

int motorOneFReverse = 74;
int motorTwoFReverse = 112;

int motorOneStop = 92;
int motorTwoStop = 92;

int motorOneBoost = 114;
int motorTwoBoost = 74;

Servo motorOne;
Servo motorTwo;

WiFiServer server(80);
String readString;

bool printSensors = false;

WiFiClient client;

class Challenge {
private:
    int FRONT_A_D_LOW = 1700; //fix
    int FRONT_A_D_HIGH = 2000; //fix
    int BACK_A_D_LOW = 2100; //fix
    int BACK_A_D_HIGH = 2200; //fix

    int FRONT_B_C_LOW = 2000; //fix
    int FRONT_B_C_HIGH = 2100; //fix
    int BACK_B_C_LOW = 2000; //fix
    int BACK_B_C_HIGH = 2100; //fix

    int SHORTER_LEFT = 0;
    int SHORTER_RIGHT = 1;
    int BUMPER_LEFT = 0;
    int BUMPER_RIGHT = 1;

    int BUMPER_LEFT_THRESHOLD = 3200;
    int BUMPER_RIGHT_THRESHOLD = 3200;

    int TWO_INCH_HIGH = 700;

    int shorterSide = -1;
    int bumperSide = -1;

    int mOneSpeed = 92;
    int mTwoSpeed = 92;
    int motorDelay = 240;

    void followSide() {
//        client.println("followSide()");
        Serial.println("followSide()");
        hitMidForward(false);

        if (shorterSide == SHORTER_LEFT) turnRight();
        else turnLeft();

        followToAD();

        followToBC();
    }

    void followToAD() {
//        client.println("followToAD()");
        Serial.println("followToAD()");

        hitMidForward(true);

        hitAD();

        if (bumperSide == BUMPER_LEFT) turnLeft();
        else turnRight();

        hitForward();
        hitMidReverse();
        turnRight();
    }

    void followToBC() {
//        client.println("followToBC()");
        Serial.println("followToBC()");

        hitBC();
        turnRight();
        hitForward();
        hitReverse();
        hitMidForward(true);
        turnRight();
        hitAD();
        turnLeft();
        hitForward();
        hitMidReverse();

        if (bumperSide == BUMPER_LEFT) turnRight();
        else turnLeft();

        hitEdge();

        if (shorterSide == SHORTER_LEFT) turnRight();
        else turnLeft();
    }

    bool hitA = false;

    void hitAD() {
//        client.println("hitAD()");
        Serial.println("hitAD()");

        if (bumperSide == BUMPER_LEFT) {
            motorOne.write(motorOneFReverse);
            motorTwo.write(motorTwoFReverse);
        } else {
            motorOne.write(mOneSpeed);
            motorTwo.write(mTwoSpeed);
        }

        while (!hitA) {
            stabilize();

            front = ultrasonicPing(FRONT);
            back = ultrasonicPing(BACK);

            if (bumperSide == BUMPER_LEFT) hitA = (back < BACK_A_D_HIGH && back > BACK_A_D_LOW);
            else hitA = (front < FRONT_A_D_HIGH && front > FRONT_A_D_LOW);
        }

        hitA = false;

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);

        delay(1000);
    }

    void hitBC() {
//        client.println("hitBC()");
        Serial.println("hitBC()");

        motorOne.write(mOneSpeed);
        motorTwo.write(mTwoSpeed);

        while (!hitA) {
            stabilize();

            front = ultrasonicPing(FRONT);
            back = ultrasonicPing(BACK);

            if (bumperSide == BUMPER_LEFT) hitA = (back < BACK_B_C_HIGH && back > BACK_B_C_LOW);
            else hitA = (front < FRONT_B_C_HIGH && front > FRONT_B_C_LOW);
        }

        hitA = false;

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);

        delay(1000);
    }

    bool hitMid = false;

    void hitMidForward(bool stabilize) {
//        client.printf("stabilize(%d)\n", stabilize);
        Serial.printf("hitMidForward(%i)\n", stabilize);

        motorOne.write(mOneSpeed);
        motorTwo.write(mTwoSpeed);

        while (!hitMid) {
            if (stabilize) this->stabilize();

            hitMid = hasHitMid();
        }

        hitMid = false;

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }


    void hitMidReverse() {
//        client.println("hitMidReverse()");
        Serial.println("hitMidReverse()");
        
        motorOne.write(motorOneFReverse);
        motorTwo.write(motorTwoFReverse);

        while (!hitMid) {
            stabilize();

            hitMid = hasHitMid();
        }

        hitMid = false;

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    bool hit = false;
    long front;

    void hitForward() {
//        client.println("hit()");
        Serial.println("hitForward()");
        
        motorOne.write(mOneSpeed);
        motorTwo.write(mTwoSpeed);

        while (!hit) {
            front = ultrasonicPing(FRONT);
            hit = front < 200 && front > 0;
        }

        hit = false;

        delay(1000);
    
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    long back;

    void hitReverse() {
//        client.println("hitReverse()");
        Serial.println("hitReverse()");
        
        motorOne.write(motorOneFReverse);
        motorTwo.write(motorTwoFReverse);

        while (!hit) {
            stabilize();

            back = ultrasonicPing(BACK);
            hit = back < 200 && back > 0;
        }

        hit = false;

        delay(1000);

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void hitEdge() {
//        client.println("hitEdge()");
        Serial.println("hitEdge()");
        
        motorOne.write(mOneSpeed);
        motorTwo.write(mTwoSpeed);

        while (!hit) {
            stabilize();

            hit = needsToStop(ultrasonicPing(FRONT));
        }

        hit = false;

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void turnLeft() {
//        client.println("turnLeft()");
        Serial.println("turnLeft()");
        
        motorOne.write(motorOneFReverse);
        motorTwo.write(mTwoSpeed);
        delay(motorDelay);
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void turnRight() {
//        client.println("turnRight()");
        Serial.println("turnRight()");
        
        motorOne.write(mOneSpeed);
        motorTwo.write(motorTwoFReverse);
        delay(motorDelay);
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    long frontLow;
    long backLow;
    long frontHigh;
    long backHigh;
    bool hasHit = false;

    bool hasHitMid() {
//        client.println("hasHitMid()");

        front = ultrasonicPing(FRONT);
        back = ultrasonicPing(BACK);

        frontLow = front - 100;
        backLow = back - 100;
        frontHigh = front + 100;
        backHigh = back + 100;

        hasHit = (back < frontHigh && back > frontLow) || (front < backHigh && front > backLow);

//        client.printf("hasHitMid() == %d\n", hasHit);

        return hasHit;
    }

    long left;
    long right;

    void stabilize() {

        left = ultrasonicPing(LEFT);
        right = ultrasonicPing(RIGHT);

        if (tooFarLeft(left, right)) {
            if (motorOne.read() > motorOneStop) {
                motorOne.write(motorOneBoost);
                delay(10);
                motorOne.write(mOneSpeed);
            }
        }

        if (tooFarRight(left, right)) {
            if (motorTwo.read() > motorTwoStop) {
                motorTwo.write(motorTwoBoost);
                delay(10);
                motorTwo.write(mTwoSpeed);
            }
        }
    }

    bool tooFarLeft(int pingLeft, int pingRight) {
        return pingLeft < pingRight;
    }

    bool tooFarRight(int pingLeft, int pingRight) {
        return pingRight < pingLeft;
    }

    bool needsToStop(int pingFront) {
        return pingFront <= TWO_INCH_HIGH;
    }

public:
    void start(int mOneSpeed, int mTwoSpeed, int delay) {
        this->mOneSpeed = mOneSpeed;
        this->mTwoSpeed = mTwoSpeed;
        this->motorDelay = delay;

        left = ultrasonicPing(LEFT);
        right = ultrasonicPing(RIGHT);

        if (left < right) shorterSide = SHORTER_LEFT;
        else shorterSide = SHORTER_RIGHT;

        if (left < right && right < BUMPER_RIGHT_THRESHOLD) bumperSide = BUMPER_RIGHT;
        else if (left < right) bumperSide = BUMPER_LEFT;
        else if (right < left && left < BUMPER_LEFT_THRESHOLD) bumperSide = BUMPER_LEFT;
        else if (right < left) bumperSide = BUMPER_RIGHT;
        
//        client.printf("mOneSpeed: %i, mTwoSpeed: %i, motorOneFReverse: %i, motorTwoFReverse: %i, turnDelay: %i, shorterSide: %i, bumperSide: %i",
//                mOneSpeed, mTwoSpeed, motorOneFReverse, motorTwoFReverse, motorDelay, shorterSide, bumperSide);

        followSide();
    }
};

Challenge challenge;

void setup() {
    Serial.begin(9600);

    pinMode(BUILTIN_LED, OUTPUT);

    motorOne.attach(MOTOR_1);
    motorTwo.attach(MOTOR_2);

    motorOne.write(MOTOR_1_STOP);
    motorTwo.write(MOTOR_2_STOP);

    WiFi.softAP("WeMOS Access Point G6", "123456789");

    server.begin();
}

void loop() {
    client = server.available();

    if (client) {
        bool currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = static_cast<char>(client.read());

                readString += c;

                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();

                    if (printSensors) {
                        client.printf("%i,%i,%i,%i\n",
                                      ultrasonicPing(LEFT),
                                      ultrasonicPing(FRONT),
                                      ultrasonicPing(RIGHT),
                                      ultrasonicPing(BACK));
                    }

                    readString = "";

                    break;
                }

                if (c == '\n') {
                    currentLineIsBlank = true;

                if (readString.indexOf("doChallenge(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        int indexComma = (unsigned) value.indexOf(',');
                        int indexColon = (unsigned) value.indexOf(':');

                        String mOne = value.substring(0, indexComma);
                        String mTwo = value.substring(indexComma + 1, indexColon);
                        String delay = value.substring(indexColon + 1, value.length());

                        challenge.start(mOne.toInt(), mTwo.toInt(), delay.toInt());
                    } else if (readString.indexOf("getSensors(") >= 0) {
                        printSensors = true;
                    }
                } else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
        }

        delay(1);
        client.stop();
    }

}