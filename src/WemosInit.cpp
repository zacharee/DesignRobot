//*************************************************************************************************************
// WemosInit.cpp - This file contains C code for "pre-defined" functions that configure and interface with the
// WeMos microcontroller used in the E121 Design I project. This file must be used in conjunction
// with header file WemosInit.h. 
// Program written by Miklos Nyary, Stevens Institute of Technology, Hoboken, NJ
// VERSION 18F-01  (August 20, 2018)  Baseline
//**************************************************************************************************************

#include "WemosInit.h"		
#include "Arduino.h"

  //The ESP8266 recognizes different pins than what is labelled on the WeMos D1 
  #if defined(d1)  //Defines Wemos D1 R1 pins to GPIO pins
    #define D0 3
    #define D1 1
    #define D2 16
    #define D8 0
    #define D9 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D10 15
  #endif 
  #if defined(d1_mini) //Defines Wemos D1 R2 pins to GPIO pins
    #define D0 16
    #define D1 5
    #define D2 4
    #define D3 0
    #define D4 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D8 15
  #endif


//This function activates the ultrasonic sensor indicated on trigPin and listens for the bounce back
//on the indicated echoPin. The function returns the time delay between pulse emission and return.
int ultrasonicPing(int trigPin,int echoPin)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  return duration;
}

int ultrasonicPing(int pin) {
    pinMode(pin, OUTPUT);

    // Clears the trigPin
    digitalWrite(pin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin, LOW);

    pinMode(pin, INPUT);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(pin, HIGH);
    return duration;
}