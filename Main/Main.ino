/*
 Name:		Main.ino
 Created:	11/10/2021 11:07:04 PM
 Author:	Aarón I. Vélez Ramírez
 
 Universidad Nacional Autónoma de México
 
 Code for Time Capsule Logger ENES León
 
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
#include "microPoly.h"


////// Comunication protocols
#include "Wire.h" // I2C


////// RTC DS3231
#include <RTClib.h>
#include <TimeLib.h>
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };


////// RTC sync over serial
#include <TimeLib.h>
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message


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






//////////////////////////////////////////
////// User Constants and Variables //////
//////////////////////////////////////////

////// Station IDs & Constants
const float VRef = 3.3;
bool debug = false;


////// Time variables
DateTime RTCnow;        // UTC Date-Time class from RTC
uint32_t local_t;       // Local time WITHOUY DST adjust in UNIX time stamp format
uint32_t NextLog = -1;  // Next time a log is due in UNIX local format
uint32_t wait_t;        // Time to next Log



////// State machine Shift Registers

int LastLog = -1;			// Last minute that variables were loged to the SD card


////// Measured variables
float BatVolt = -1;    // Battery voltage
float Temp = -1;        // Temperature

////// EEPROM variables
uint32_t EEPROM_Addres = 0;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {

    ////// Setup PINs
    pinMode(WakeUp_PIN, INPUT_PULLUP);
    pinMode(Sleep_PIN, OUTPUT);
    digitalWrite(Sleep_PIN, LOW);


    ////// Start RTC
    rtc.begin();
    rtc.disable32K();                   // Disable the 32K pin
    rtc.clearAlarm(1);                  // reset rtc alarm flag
    rtc.disableAlarm(2);                // stop alarm 2
    rtc.writeSqwPinMode(DS3231_OFF);    // Stop oscillating signat at rtc SQW pin
    setSyncProvider(requestSync); // Set function to call when date and time sync required


    ///// Start EEPROM
    myEEPROM.begin();


	////// Test time
    /// Get time
    RTCnow = rtc.now();
    setTime(RTCnow.hour,
        RTCnow.minute,
        RTCnow.second,
        RTCnow.day,
        RTCnow.month,
        RTCnow.year);
    local_t = now();

    /// Get Next Log time from EEPROM
    myEEPROM.get(0, NextLog);

    /// Test what to do depending on remaining waiting time
    wait_t = NextLog - local_t;
    if (wait_t > 6750) {
        // Turn OFF
        digitalWrite(Sleep_PIN, HIGH);
        delay(1000);
    }
    else if (wait_t > 5) {
        // Set RTC alarm
        rtc.setAlarm1(rtc.now() + TimeSpan(wait_t - 2),
            DS3231_A1_Hour);    // Alarm when hour, minute and seconds match (DS3231_A1_Hour mode)
        // Put uHEX to sleep
        PolyuHex.addPinTrigger(WakeUp_PIN, LOW);    // Set up uHEX wake up condition
        PolyuHex.sleep();                           // Set uHEx to sleep
    }
    

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
    setTime(RTCnow.hour,
        RTCnow.minute,
        RTCnow.second,
        RTCnow.day,
        RTCnow.month,
        RTCnow.year);
    local_t = now();


    ////// State 2. Test if it is time to log
    if (local_t == NextLog) {
        Meas_Rec_Sleep();
    }
    else if (local_t > NextLog) {
        if (local_t % 43200 == 0) {
        // time to log
            Meas_Rec_Sleep();
        }
        else {

        }
    }

    ////// State 3. Wait
    delay(250);
  
}
