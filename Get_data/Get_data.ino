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
float BatVolt = -1;     // Battery voltage
float Temp = -1;        // Temperature


////// EEPROM variables
uint32_t EEPROM_Size = 0;
uint32_t EEPROM_Addres = 0;


//////////////////////////////////////////////////////////////////////////
// the setup function runs once when you press reset or power the board //
//////////////////////////////////////////////////////////////////////////
void setup() {
    // Start serial
    Serial.begin(115200);


    ////// Start I2C
    Wire.begin();


    ///// Start EEPROM
    myEEPROM.begin(0x54);


    // continue to loop 
}

//////////////////////////////////////////////////////////////////////////
// The loop function runs over and over again until power down or reset //
//////////////////////////////////////////////////////////////////////////
void loop() {

    ////// Check if Serial Monitor is ready to recieve data
    if (Serial.available() == 'S') {
        // Start EEPROM 1
        myEEPROM.begin(0x50);
        Serial.println("EEPROM 1 data:");
        // Get EEPROM 1 data and send via serial
        Get_Data();

        // Start EEPROM 2
        myEEPROM.begin(0x54);
        Serial.println("EEPROM 2 data:");
        // Get EEPROM 1 data and send via serial
        Get_Data();        
    }


}


void Get_Data() {
    EEPROM_Size = myEEPROM.getMemorySize();
    EEPROM_Addres = 8;
    while (EEPROM_Addres < EEPROM_Size) {
        // Print EEPROM address of current record
        Serial.print(EEPROM_Addres);
        Serial.print("\t");
        //Get and print UNIX time of current record
        myEEPROM.get(EEPROM_Addres, local_t);
        Serial.print(local_t);
        Serial.print("\t");
        // Get and print Temperature of current record
        EEPROM_Addres += 4;
        myEEPROM.get(EEPROM_Addres, Temp);
        Temp = Temp / 10000;
        Serial.print(Temp);
        Serial.print("\t");
        // Get and print Battery voltage of current record
        EEPROM_Addres += 2;
        myEEPROM.get(EEPROM_Addres, BatVolt);
        BatVolt = BatVolt / 10000;
        Serial.print(BatVolt);
        Serial.print("\n");
        // Update EEPROM address for next recod            
        EEPROM_Addres += 2;
    }
    Serial.print("Done, bytes read: ");
    Serial.print(EEPROM_Addres);
}