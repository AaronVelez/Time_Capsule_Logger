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

*/





////////////////////////////////////////
// Libraries and associated constants //
////////////////////////////////////////

////// Low Power library
//#include <microPoly.h>    // Use LowPower library instead of uHex library
#include <LowPower.h>


////// Comunication protocols
#include <Wire.h> // I2C


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
    rtc.writeSqwPinMode(DS3231_OFF);    // Stop oscillating signal at rtc SQW pin

                                        
    ////// Test time
    /// Get time
    RTCnow = rtc.now();
    local_t = RTCnow.unixtime();


    ///// Start EEPROM
    myEEPROM.begin(0x54);
	

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
    local_t = RTCnow.unixtime();


    ////// State 2. Test if it is time to log
    if (local_t == NextLog) {
        Meas_Rec_Sleep();
    }
    // Log time has passed
    else if (local_t > NextLog) {
        Calculate_NextLog();
        Set_Alarm_Sleep();
        }


    ////// State 3. Just in case, Test time and conitnue OR set alarm (if needed), then sleep.
    Set_Alarm_Sleep();
    

    ////// State 4. Wait
    delay(200);

    }