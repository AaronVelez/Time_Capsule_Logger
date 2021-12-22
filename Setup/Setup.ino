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
//#include "microPoly.h"


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
uint32_t testRead = 0;


////// SHT35
#include <DFRobot_SHT3x.h>
DFRobot_SHT3x sht3x(&Wire,/*address=*/0x45,/*RST=*/4);
DFRobot_SHT3x::sRHAndTemp_t sht3x_data;
float Temp = 0;
float RH = 0;


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


////// Time variables
DateTime RTCnow;        // UTC Date-Time class from RTC

////// EEPROM variables
uint32_t NextLog = 0;
uint32_t EEPROM_Address = 8;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {

    Serial.begin(115200);
    delay(2000);
    Wire.begin();
    

    //// Start Temp sensor
    Serial.print("Temp sensor begin: ");
    Serial.println(sht3x.begin());
    Serial.print("Temp sensor reset: ");
    Serial.println(sht3x.softReset());


    ////// Start RTC
    Serial.println("Starting RTC...");
    Serial.print("Begin status: ");
    Serial.println(rtc.begin());
    rtc.disable32K();                   // Disable the 32K pin
    rtc.clearAlarm(1);                  // reset rtc alarm flag
    rtc.disableAlarm(2);                // stop alarm 2
    rtc.writeSqwPinMode(DS3231_OFF);    // Stop oscillating signat at rtc SQW pin
    setSyncProvider(requestSync); // Set function to call when date and time sync required


    ////// Setup EEPROMs
    Serial.println("Starting EEPROM 1...");
    Serial.print("Success: ");
    Serial.println(myEEPROM.begin(0x54));
    // Erase EEPROM
    Serial.println("Erasing EEPROM 1...");
    myEEPROM.erase();
    // Test is blank
    Serial.println("Erase done");
    if (myEEPROM.get(0, testRead) == 0) {
        Serial.println("Erase successful");
    }
    else {
        Serial.println("Erase NOT successful");
    }
    // Write Nextlog register
    Serial.println("Reseting EEPROM 1 registers...");
    NextLog = 0;
    myEEPROM.put(0, NextLog);
    while (myEEPROM.isBusy()) { delay(2); }
    Serial.println("Reading NextLog register...");
    myEEPROM.get(0, NextLog);
    Serial.print("NextLog: ");
    Serial.println(NextLog);
    // Write EEPROM_Address register
    EEPROM_Address = 8;
    myEEPROM.put(4, EEPROM_Address);
    while (myEEPROM.isBusy()) { delay(2); }
    Serial.println("Reading EEPROM_Address register...");
    myEEPROM.get(4, EEPROM_Address);
    Serial.print("EEPROM_Address: ");
    Serial.println(EEPROM_Address);


    ////// continue to loop 
    Serial.println("Setup finished");
}




//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {
    ////// State 0. Syns RTC If valid time mesage is available.
    if (Serial.available()) {
        Serial.println(F("Serial data recieved"));
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


    ////// State 3. Read temp
    sht3x_data = sht3x.readTemperatureAndHumidity(sht3x.eRepeatability_High);
    Temp = sht3x_data.TemperatureC;
    RH = sht3x_data.Humidity;
    Serial.print("Temperature: ");
    Serial.println(Temp);


    ////// State 4. Wait
    delay(1000);
}
