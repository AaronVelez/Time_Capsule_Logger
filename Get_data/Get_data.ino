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
uint32_t local_t;       // Local time WITHOUY DST adjust in UNIX time stamp format


////// Measured variables
uint16_t EEPROM_BatVolt = -1;    // Battery voltage in EEPROM
uint16_t EEPROM_Temp = -1;        // Temperature in EEPROM
float BatVolt = -1;         // Battery voltage
float Temp = -1;            // Temperature


////// EEPROM variables
uint32_t NextLog = 0;
uint32_t EEPROM_Size = 0;
uint32_t EEPROM_Address = 0;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {
    // Start serial
    Serial.begin(115200);
    delay(100);
    Serial.println(F("Serial com started"));


    ////// Start I2C
    Wire.begin();
    Serial.println(F("Wire com started"));

    
    ///// Start EEPROM
    Serial.println("Starting EEPROM...");
    Serial.print("Success: ");
    Serial.println(myEEPROM.begin(0x54));
    

    ////// Read EEPROM header
    myEEPROM.get(0, NextLog);
    Serial.print(F("Next Log in address 0: "));
    Serial.println(NextLog);

    myEEPROM.get(4, EEPROM_Address);
    Serial.print(F("EEPROM_Address in address 4: "));
    Serial.println(EEPROM_Address);
    
    myEEPROM.get(8, local_t);
    Serial.print(F("Local time at start: "));
    Serial.println(local_t);



    // continue to loop 
    Serial.println(F("Setup finished"));
}

//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {

    ////// Check if Serial Monitor is ready to recieve data
    if (Serial.find('S')) {
        Serial.println("EEPROM data:");
        // Get EEPROM 1 data and send via serial
        Get_Data();
   
    }


}


void Get_Data() {
    EEPROM_Size = myEEPROM.getMemorySize();
    EEPROM_Address = 8;
    while (EEPROM_Address < EEPROM_Size) {
        // Print EEPROM address of current record
        Serial.print(EEPROM_Address);
        Serial.print("\t");
        //Get and print UNIX time of current record
        myEEPROM.get(EEPROM_Address, local_t);
        Serial.print(local_t);
        Serial.print("\t");
        // Get and print Temperature of current record
        EEPROM_Address += 4;
        myEEPROM.get(EEPROM_Address, EEPROM_Temp);
        Temp = (float)EEPROM_Temp / 1000;
        Serial.print(Temp, 3);
        Serial.print("\t");
        // Get and print Battery voltage of current record
        EEPROM_Address += 2;
        myEEPROM.get(EEPROM_Address, EEPROM_BatVolt);
        BatVolt = (float)EEPROM_BatVolt / 1000;
        Serial.print(BatVolt, 3);
        Serial.print("\n");
        // Update EEPROM address for next recod            
        EEPROM_Address += 2;
    }
    Serial.print("Done, bytes read: ");
    Serial.print(EEPROM_Address);
}