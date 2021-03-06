/*
 Name:		Main.ino
 Created:	11/10/2021 11:07:04 PM
 Author:	Aar?n I. V?lez Ram?rez

 Universidad Nacional Aut?noma de M?xico

 Code for Time Capsule Logger ENES Le?n

 Hardware details:

 Main board is DFRobor uHex
 Power cycle management with SparkFun TPL5110
 Voltage regulation with Sparkfun LiPower - Boost Converter
 Time keep with DFRobot RTC DS3231
 Temperature sensor with DFRobot SHT35
 Memory starage with SparkFun Qwiic EEPROM Breakout - 512Kbit


 Software details

 Time adjust via Serial
 Deep sleep board wakeup with RTC alarm

*/





////////////////////////////////////////
// Libraries and associated constants //
////////////////////////////////////////

////// uHex board library
//#include "microPoly.h"


// NEW!
#include "LowPower.h"

////// Comunication protocols
#include "Wire.h" // I2C


////// RTC DS3231
#include <RTClib.h>
#include <TimeLib.h>
RTC_DS3231 rtc;


////// SHT35
#include <DFRobot_SHT3x.h>
DFRobot_SHT3x sht3x(&Wire,/*address=*/0x45,/*RST=*/4);
DFRobot_SHT3x::sRHAndTemp_t sht3x_data;

////// EEPROM
#include <SparkFun_External_EEPROM.h>
ExternalEEPROM myEEPROM;


//////////
// Pins //
//////////
int Battery_PIN = A3;
int WakeUp_PIN = 3;
int Sleep_PIN = 11;
int MOSFET_PIN = 2;





//////////////////////////////////////////
////// User Constants and Variables //////
//////////////////////////////////////////

////// Station IDs & Constants
const float VRef = 3.3;


////// Time variables
DateTime RTCnow;        // UTC Date-Time class from RTC
uint32_t local_t;       // Local time WITHOUY DST adjust in UNIX time stamp format
uint32_t local_t_Days;  // Days since UNIX EPOCH in local time
bool EvenDay = true;    // Register if present day is even or odd
uint32_t NextLog = -1;  // Next time a log is due in UNIX local format
long wait_t;        // Time to next Log


////// Measured variables
float BatVolt = -1;     // Battery voltage
float Temp = -1;        // Temperature
const int n = 10;      // measure n times the ADC input for averaging
float sum = 0;          // shift register to hold ADC data

////// EEPROM variables
uint32_t EEPROM_Address = 0;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("Serial comunication started"));
    Serial.println(F("Debug version 3.0"));

    ////// Start I2C
    Wire.begin();


    ////// Setup PINs
    pinMode(WakeUp_PIN, INPUT_PULLUP);
    pinMode(Sleep_PIN, OUTPUT);
    digitalWrite(Sleep_PIN, LOW);
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);


    ////// Start RTC
    rtc.begin();
    rtc.disable32K();                   // Disable the 32K pin
    rtc.clearAlarm(1);                  // reset rtc alarm flag
    rtc.disableAlarm(2);                // stop alarm 2
    rtc.writeSqwPinMode(DS3231_OFF);    // Stop oscillating signat at rtc SQW pin


    ////// Test time
    /// Get time
    RTCnow = rtc.now();
    local_t = RTCnow.unixtime();
    Serial.print(F("RTC time: "));
    Serial.println(local_t);


    ///// Start EEPROM
    myEEPROM.begin(0x54);


    /// Get Next Log time from EEPROM
    myEEPROM.get(0, NextLog);
    Serial.print(F("Next Log time: "));
    Serial.println(NextLog);


   
    ///// Start SHT35 Temp and RH sensor
    sht3x.begin();
    sht3x.softReset();









    // continue to loop
}

//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {
    /////// State 1. Get time
    RTCnow = rtc.now();
    local_t = RTCnow.unixtime();
    Serial.print(F("Loop start at: "));
    Serial.println(local_t);
    Serial.print(F("Next Log: "));
    Serial.println(NextLog);



    ////// TEST BUG
    Serial.println(F("Setting wakeup alarm"));
    rtc.clearAlarm(1);  // NEW!

    DateTime NowTest = (rtc.now() + TimeSpan(20));
    Serial.print(F("Test alarm time is: "));
    Serial.print(NowTest.year(), DEC);
    Serial.print('/');
    Serial.print(NowTest.month(), DEC);
    Serial.print('/');
    Serial.print(NowTest.day(), DEC);
    Serial.print(" (");
    Serial.print(NowTest.hour(), DEC);
    Serial.print(':');
    Serial.print(NowTest.minute(), DEC);
    Serial.print(':');
    Serial.print(NowTest.second(), DEC);
    Serial.print(')');
    Serial.println();

    Serial.print(F("Setting alarm success: "));
    Serial.println(rtc.setAlarm1(rtc.now() + TimeSpan(20), DS3231_A1_Hour));
    
    
    Serial.println(F("Putting uHex to sleep"));
    Serial.println();
    delay(1000);

    // NEW!
    attachInterrupt(digitalPinToInterrupt(WakeUp_PIN), wakeUp, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    detachInterrupt(digitalPinToInterrupt(WakeUp_PIN));

    // Put uHEX to sleep
    //Serial.print(F("Pin triggered? "));
    //Serial.println(PolyuHex.isPinTriggered());
    //delay(1000);

    //PolyuHex.sleep();                           // Set uHEx to sleep

    Serial.println(F("Waked up from sleep by RTC alarm"));









    ////// State 3. Wait
    delay(10000);


    

}


// NEW!
void wakeUp()
{
    // Just a handler for the pin interrupt.
}