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
int MOSFET_PIN = 10;





//////////////////////////////////////////
////// User Constants and Variables //////
//////////////////////////////////////////

////// Station IDs & Constants
const float VRef = 3.3;


////// Time variables
DateTime RTCnow;        // UTC Date-Time class from RTC
uint32_t local_t;       // Local time WITHOUY DST adjust in UNIX time stamp format
uint32_t NextLog = -1;  // Next time a log is due in UNIX local format
uint32_t wait_t;        // Time to next Log


////// Measured variables
float BatVolt = -1;     // Battery voltage
float Temp = -1;        // Temperature
const int n = 100;      // measure n times the ADC input for averaging
float sum = 0;          // shift register to hold ADC data

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
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);


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
    setTime(RTCnow.hour(),
        RTCnow.minute(),
        RTCnow.second(),
        RTCnow.day(),
        RTCnow.month(),
        RTCnow.year());
    local_t = now();

    /// Get Next Log time from EEPROM
    myEEPROM.get(0, NextLog);

    
    /// Test time and conitnue OR set alarm (if needed), then sleep.
    Set_Alarm_Sleep();
    

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
    setTime(RTCnow.hour(),
        RTCnow.minute(),
        RTCnow.second(),
        RTCnow.day(),
        RTCnow.month(),
        RTCnow.year());
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
            for (uint32_t i = local_t; i % 200 == 0; i++) {
                NextLog = i;
            }
            for (uint32_t i = NextLog; i % 43200 == 0; i += 200) {
                NextLog = i;
            }
            myEEPROM.put(0, NextLog);
            while (myEEPROM.isBusy()) { delay(2); }

            Set_Alarm_Sleep();
        }
    }

    ////// State 3. Wait
    delay(200);
  
}
