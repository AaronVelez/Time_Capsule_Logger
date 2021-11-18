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


////// EEPROM
#include <SparkFun_External_EEPROM.h>
ExternalEEPROM myEEPROM;


//////////
// Pins //
//////////






//////////////////////////////////////////
////// User Constants and Variables //////
//////////////////////////////////////////


////// Time variables
DateTime RTCnow;        // UTC Date-Time class from RTC





//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {

    Serial.begin(115200);
    delay(5000);


    ////// Start RTC
    Serial.println("Starting RTC...");
    rtc.begin();
    rtc.disable32K();                   // Disable the 32K pin
    rtc.clearAlarm(1);                  // reset rtc alarm flag
    rtc.disableAlarm(2);                // stop alarm 2
    rtc.writeSqwPinMode(DS3231_OFF);    // Stop oscillating signat at rtc SQW pin
    setSyncProvider(requestSync); // Set function to call when date and time sync required


    ///// Start EEPROM
    Serial.println("Starting EEPROM...");
    myEEPROM.begin();


    ////// Erase EEPROM
    Serial.println("Erasing EEPROM...");
    myEEPROM.erase(0x00);
    while (myEEPROM.isBusy()) { delay(2); }
    

    ////// Write EEPROM registers
    Serial.println("Reseting EEPROM registers...");
    uint32_t NextLog = 0;
    myEEPROM.put(0, NextLog);
    while (myEEPROM.isBusy()) { delay(2); }
    uint32_t EEPROM_Address = 8;
    myEEPROM.put(4, EEPROM_Address);
    while (myEEPROM.isBusy()) { delay(2); }


    ////// continue to loop 
    Serial.println("Setu finished");
}

//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {
    ////// State 0. Syns RTC If valid time mesage is available.
    if (Serial.available()) {
        processSyncMessage();
    }


    /////// State 2. Get time
    RTCnow = rtc.now();

    Serial.print(RTCnow.year(), DEC);
    Serial.print('/');
    Serial.print(RTCnow.month(), DEC);
    Serial.print('/');
    Serial.print(RTCnow.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[RTCnow.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(RTCnow.hour(), DEC);
    Serial.print(':');
    Serial.print(RTCnow.minute(), DEC);
    Serial.print(':');
    Serial.print(RTCnow.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(RTCnow.unixtime());
    Serial.print("s = ");
    Serial.print(RTCnow.unixtime() / 86400L);
    Serial.println("d");


    ////// State 3. Wait
    delay(1000);
}
