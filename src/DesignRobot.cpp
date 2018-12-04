#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include "WemosInit.h"

uint8_t MOTOR_1 = 4;
uint8_t MOTOR_2 = 14;

int GOOD_TURN_DELAY = 240;

int SIDE_DISTANCE_RIGHT_LOW = 400; //fix
int SIDE_DISTANCE_RIGHT_HIGH = 500; //fix
int SIDE_DISTANCE_LEFT_LOW = 400; //fix
int SIDE_DISTANCE_LEFT_HIGH = 500; //fix

int FRONT_BACK_DIST_MID_LONG = 3800; //fix
int FRONT_BACK_DIST_MID_SHORT = 1400; //fix

//uint8_t ECHO_LEFT = 12;
//uint8_t TRIG_LEFT = 13;
//uint8_t ECHO_FRONT = 0;
//uint8_t TRIG_FRONT = 15;
//uint8_t ECHO_RIGHT = 16;
//uint8_t TRIG_RIGHT = 5;

uint8_t LEFT = 12;
uint8_t FRONT = 13;
uint8_t RIGHT = 15;
uint8_t BACK = 16;

int TWO_INCH_HIGH = 700;

int MOTOR_1_STOP = 92;
int MOTOR_2_STOP = 92;

int motorOneFReverse = 74;
int motorTwoFReverse = 112;

int motorOneStop = 92;
int motorTwoStop = 92;

int motorOneFull = 110;
int motorTwoFull = 77;

int motorOneBoost = 114;
int motorTwoBoost = 74;

Servo motorOne;
Servo motorTwo;

WiFiServer server(80);
String readString;

bool printLedState = false;
bool printSensors = false;
int printMotorState = -1;

bool runStraightStop = false;
bool runRunNascar = false;
bool runChallenge = false;

int turnCount = 0;
bool currentlyStopped = false;

int mOne;
int mTwo;
int mDelay;

void setMotorAbsolute(int which, int speed);
void setMotor(int which, signed int percent);
int getMotor(int which);
void serialFlush();
void toggleLed(uint8_t value);
bool isTooClose(int ping);
bool isMotorRunningForward(uint8_t motor);
bool isMotorRunningBackwards(uint8_t motor);
bool tooFarLeft(int pingLeft, int pingRight);
bool tooFarRight(int pingLeft, int pingRight);
bool needsToStop(int pingFront);
void runNascar(int mOne, int mTwo, int delay);
void turnLeft(int turnDelay);
void straightStop(int mOne, int mTwo);

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

    int shorterSide = -1;
    int bumperSide = -1;

    int mOneSpeed;
    int mTwoSpeed;
    int motorDelay;

    void followSide() {
        hitMid(false);

        if (shorterSide == SHORTER_LEFT) turnRight();
        else turnLeft();

        followToAD();

        followToBC();
    }

    void followToAD() {
        hitMid(true);

        hitAD();

        if (bumperSide == BUMPER_LEFT) turnLeft();
        else turnRight();

        hit();
        hitMidReverse();
        turnRight();
    }

    void followToBC() {
        hitBC();
        turnRight();
        hit();
        hitReverse();
        hitMid(true);
        turnRight();
        hitAD();
        turnLeft();
        hit();
        hitMidReverse();

        if (bumperSide == BUMPER_LEFT) turnRight();
        else turnLeft();

        hitEdge();

        if (shorterSide == SHORTER_LEFT) turnRight();
        else turnLeft();
    }

    void hitAD() {
        if (bumperSide == BUMPER_LEFT) {
            motorOne.write(motorOneFReverse);
            motorTwo.write(motorTwoFReverse);
        } else {
            motorOne.write(motorOneFull);
            motorTwo.write(motorTwoFull);
        }

        bool hitA = false;

        while (!hitA) {
            stabilize();

            auto front = ultrasonicPing(FRONT);
            auto back = ultrasonicPing(BACK);

            if (bumperSide == BUMPER_LEFT) hitA = (back < BACK_A_D_HIGH && back > BACK_A_D_LOW);
            else hitA = (front < FRONT_A_D_HIGH && front > FRONT_A_D_LOW);
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);

        delay(1000);
    }

    void hitBC() {
        motorOne.write(motorOneFull);
        motorTwo.write(motorTwoFull);

        bool hitA = false;

        while (!hitA) {
            stabilize();

            auto front = ultrasonicPing(FRONT);
            auto back = ultrasonicPing(BACK);

            if (bumperSide == BUMPER_LEFT) hitA = (back < BACK_B_C_HIGH && back > BACK_B_C_LOW);
            else hitA = (front < FRONT_B_C_HIGH && front > FRONT_B_C_LOW);
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);

        delay(1000);
    }

    void hitMid(bool stabilize) {
        motorOne.write(motorOneFull);
        motorTwo.write(motorTwoFull);

        bool hitMid = false;

        while (!hitMid) {
            if (stabilize) this->stabilize();

            hitMid = hasHitMid();
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }


    void hitMidReverse() {
        motorOne.write(motorOneFReverse);
        motorTwo.write(motorTwoFReverse);

        bool hitMid = false;

        while (!hitMid) {
            stabilize();

            hitMid = hasHitMid();
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void hit() {
        motorOne.write(motorOneFull);
        motorTwo.write(motorTwoFull);

        bool hit = false;

        while (!hit) {
            auto front = ultrasonicPing(FRONT);
            hit = front < 100;
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void hitReverse() {
        motorOne.write(motorOneFReverse);
        motorTwo.write(motorTwoFReverse);

        bool hit = false;

        while (!hit) {
            stabilize();

            auto back = ultrasonicPing(BACK);
            hit = back < 100;
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void hitEdge() {
        motorOne.write(motorOneFull);
        motorTwo.write(motorTwoFull);

        bool hit = false;

        while (!hit) {
            stabilize();

            hit = needsToStop(ultrasonicPing(FRONT));
        }

        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void turnLeft() {
        motorOne.write(motorOneFReverse);
        motorTwo.write(motorTwoFull);
        delay(motorDelay);
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    void turnRight() {
        motorOne.write(motorOneFull);
        motorTwo.write(motorTwoFReverse);
        delay(motorDelay);
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        delay(1000);
    }

    bool hasHitMid() {
        auto front = ultrasonicPing(FRONT);
        auto back = ultrasonicPing(BACK);

        auto frontLow = front - 100;
        auto backLow = back - 100;
        auto frontHigh = front + 100;
        auto backHigh = back + 100;

        auto hasHit = (back < frontHigh && back > frontLow) || (front < backHigh && front > backLow);

        Serial.printf("Front: %d, Low: %d, High: %d, Back: %d, Low: %d, High: %d, Hit: %d \n",
                      front, frontLow, frontHigh, back, backLow, backHigh, hasHit);

        return hasHit;
    }

    void stabilize() {
        int pingLeft = ultrasonicPing(LEFT);
        int pingRight = ultrasonicPing(RIGHT);

        if (tooFarLeft(pingLeft, pingRight)) {
            if (isMotorRunningForward(MOTOR_1)) {
                motorOne.write(motorOneBoost);
                delay(10);
                motorOne.write(motorOneFull);
            }
        }

        if (tooFarRight(pingLeft, pingRight)) {
            if (isMotorRunningForward(MOTOR_2)) {
                motorTwo.write(motorTwoBoost);
                delay(10);
                motorTwo.write(motorTwoFull);
            }
        }
    }

public:
    void start(int mOneSpeed, int mTwoSpeed, int delay) {
        this->mOneSpeed = mOneSpeed;
        this->mTwoSpeed = mTwoSpeed;
        this->motorDelay = delay;

        auto left = ultrasonicPing(LEFT);
        auto right = ultrasonicPing(RIGHT);

        if (left < right) shorterSide = SHORTER_LEFT;
        else shorterSide = SHORTER_RIGHT;

        if (left < BUMPER_LEFT_THRESHOLD) bumperSide = BUMPER_LEFT;
        else if (right < BUMPER_RIGHT_THRESHOLD) bumperSide = BUMPER_RIGHT;
        else if (left < right) bumperSide = BUMPER_LEFT;
        else bumperSide = BUMPER_RIGHT;

        followSide();
    }
};

Challenge challenge;

void setup() {
    Serial.begin(9600);

    pinMode(BUILTIN_LED, OUTPUT);

    toggleLed(LOW);

    motorOne.attach(MOTOR_1);
    motorTwo.attach(MOTOR_2);

    motorOne.write(MOTOR_1_STOP);
    motorTwo.write(MOTOR_2_STOP);

    WiFi.softAP("WeMOS Access Point G6", "123456789");

    server.begin();
}

void loop() {
    WiFiClient client = server.available();

//    if (runStraightStop) {
//        straightStop(mOne, mTwo);
//    }
//
//    if (runRunNascar) {
//        runNascar(mOne, mTwo, mDelay);
//    }

    Serial.print("Left: ");
    Serial.print(ultrasonicPing(LEFT));
    Serial.print(", Front: ");
    Serial.print(ultrasonicPing(FRONT));
    Serial.print(", Right: ");
    Serial.print(ultrasonicPing(RIGHT));
    Serial.print(", Back: ");
    Serial.println(ultrasonicPing(BACK));


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
//          client.println("<!DOCTYPE HTML>");
//          client.println("<html>");

                    if (printLedState) {
                        client.println(digitalRead(BUILTIN_LED));
                        printLedState = false;
                    }

                    if (printMotorState != -1) {
                        client.println(getMotor(printMotorState));
                        printMotorState = -1;
                    }

                    if (printSensors) {
                        client.printf("%i,%i,%i,%i\n",
                                      ultrasonicPing(LEFT),
                                      ultrasonicPing(FRONT),
                                      ultrasonicPing(RIGHT),
                                      ultrasonicPing(BACK));
                    }

//          client.println("</html>");

                    readString = "";

                    break;
                }

                if (c == '\n') {
                    currentLineIsBlank = true;

                    if (readString.indexOf("toggleLed(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));
                        toggleLed((uint8_t) value.toInt());
                    } else if (readString.indexOf("setMotor(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        String motor = value.substring(0, (unsigned) value.indexOf(','));
                        String speed = value.substring((unsigned) value.indexOf(',') + 1, value.length());

                        setMotorAbsolute(motor.toInt(), speed.toInt());
                    } else if (readString.indexOf("setBothMotors(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        String motorOne = value.substring(0, (unsigned) value.indexOf(','));
                        String motorTwo = value.substring((unsigned) value.indexOf(',') + 1, value.length());

                        setMotorAbsolute(1, motorOne.toInt());
                        setMotorAbsolute(2, motorTwo.toInt());

                        if (motorOne.toInt() == MOTOR_1_STOP && motorTwo.toInt() == MOTOR_2_STOP) {
                            runRunNascar = false;
                            runStraightStop = false;
                            turnCount = 0;
                            currentlyStopped = false;
                        }
                    } else if (readString.indexOf("getLedState(") >= 0) {
                        printLedState = true;
                    } else if (readString.indexOf("getMotorState(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        printMotorState = value.toInt();
                    } else if (readString.indexOf("setMotorPercent(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        String motor = value.substring(0, (unsigned) value.indexOf(','));
                        String speed = value.substring((unsigned) value.indexOf(',') + 1, value.length());

                        int speedInt = speed.toInt();

                        setMotor(motor.toInt(), speedInt);
                    } else if (readString.indexOf("straightStop(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        String mOne = value.substring(0, (unsigned) value.indexOf(','));
                        String mTwo = value.substring((unsigned) value.indexOf(',') + 1, value.length());

                        if (!runStraightStop) straightStop(mOne.toInt(), mTwo.toInt());
                    } else if (readString.indexOf("runNascar(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        auto indexComma = (unsigned) value.indexOf(',');
                        auto indexColon = (unsigned) value.indexOf(':');

                        String mOne = value.substring(0, indexComma);
                        String mTwo = value.substring(indexComma + 1, indexColon);
                        String delay = value.substring(indexColon + 1, value.length());

                        if (!runRunNascar) runNascar(mOne.toInt(), mTwo.toInt(), delay.toInt());
                    } else if (readString.indexOf("doChallenge(") >= 0) {
                        String value = readString.substring((unsigned) readString.indexOf("((", 10) + 2,
                                                            (unsigned) readString.indexOf("))", 10));

                        auto indexComma = (unsigned) value.indexOf(',');
                        auto indexColon = (unsigned) value.indexOf(':');

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

bool isTooClose(int ping) {
    return ping < 300;
}

bool isMotorRunningForward(uint8_t motor) {
    if (motor == MOTOR_1) return motorOne.read() > MOTOR_1_STOP;
    else return motorTwo.read() < MOTOR_2_STOP;
}

bool isMotorRunningBackwards(uint8_t motor) {
    if (motor == MOTOR_1) return motorOne.read() < MOTOR_1_STOP;
    else return motorTwo.read() > MOTOR_2_STOP;
}

bool tooFarLeft(int pingLeft, int pingRight) {
    return pingLeft < pingRight;
}

bool tooFarRight(int pingLeft, int pingRight) {
    return pingRight < pingLeft;
}

void setMotorAbsolute(int which, int speed) {
    if (which == 1) motorOne.write(speed);
    else if (which == 2) motorTwo.write(speed);
}

int getMotor(int which) {
    if (which == 1) return motorOne.read();
    else if (which == 2) return motorTwo.read();
    else return -1;
}

void serialFlush() {
    while (Serial.available() > 0) {
        char t = static_cast<char>(Serial.read());
    }
}

void toggleLed(uint8_t value) {
    digitalWrite(BUILTIN_LED, value);
}

void setMotorOne(int percent) {
    setMotor(1, percent);
}

void setMotorTwo(int percent) {
    setMotor(2, percent);
}

void setMotor(int which, signed int percent) {
    if (which == 1) {
        if (percent == 0) motorOne.write(motorOneStop);
        else motorOne.write(map(percent, -100, 100, motorOneFReverse, motorOneFull));
    } else if (which == 2) {
        percent = -percent;
        if (percent == 0) motorTwo.write(motorTwoStop);
        else motorTwo.write(map(percent, -100, 100, motorTwoFull, motorTwoFReverse));
    }
}

bool needsToStop(int pingFront) {
    return pingFront <= TWO_INCH_HIGH;
}

void straightStop(int mOne, int mTwo) {
    if (!runStraightStop) {
        runStraightStop = true;
        ::mOne = mOne;
        ::mTwo = mTwo;
    }

    motorOne.write(mOne);
    motorTwo.write(mTwo);

    int pingLeft = ultrasonicPing(LEFT);
    int pingFront = ultrasonicPing(FRONT);
    int pingRight = ultrasonicPing(RIGHT);

    if (tooFarLeft(pingLeft, pingRight)) {
        if (isMotorRunningForward(MOTOR_1)) {
            motorOne.write(motorOneBoost);
            delay(10);
            motorOne.write(motorOneFull);
        }
    }

    if (tooFarRight(pingLeft, pingRight)) {
        if (isMotorRunningForward(MOTOR_2)) {
            motorTwo.write(motorTwoBoost);
            delay(10);
            motorTwo.write(motorTwoFull);
        }
    }

    if (needsToStop(pingFront)) {
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
    }
}

void runNascar(int mOne, int mTwo, int turnDelay) {
    if (!runRunNascar) {
        runRunNascar = true;
        ::mOne = mOne;
        ::mTwo = mTwo;
        ::mDelay = turnDelay;
    }

    motorOne.write(mOne);
    motorTwo.write(mTwo);

    int pingLeft = ultrasonicPing(LEFT);
    int pingFront = ultrasonicPing(FRONT);
    int pingRight = ultrasonicPing(RIGHT);

    if (tooFarLeft(pingLeft, pingRight)) {
        if (isMotorRunningForward(MOTOR_1)) {
            motorOne.write(motorOneBoost);
            delay(10);
            motorOne.write(motorOneFull);
        }
    }

    if (tooFarRight(pingLeft, pingRight)) {
        if (isMotorRunningForward(MOTOR_2)) {
            motorTwo.write(motorTwoBoost);
            delay(10);
            motorTwo.write(motorTwoFull);
        }
    }

    if (needsToStop(pingFront) && !currentlyStopped) {
        currentlyStopped = true;
        motorOne.write(motorOneStop);
        motorTwo.write(motorTwoStop);
        if (turnCount < 4) {
            delay(100);
            turnLeft(turnDelay);
            turnCount++;
            motorOne.write(motorOneStop);
            motorTwo.write(motorTwoStop);
            delay(100);
        } else {
            runRunNascar = false;
            turnCount = 0;
        }
    } else {
        currentlyStopped = false;
    }
}

void turnLeft(int turnDelay) {
    motorOne.write(motorOneFReverse);
    motorTwo.write(motorTwoFull);
    delay((unsigned long) turnDelay);
}