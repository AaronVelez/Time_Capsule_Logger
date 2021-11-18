////// Test time, then deepsleep, sleep or continue to loop
void Set_Alarm_Sleep() {
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
}


////// Take temperature and battery voltage data, store it in EEPROM, and put uHEX to sleep
void Meas_Rec_Sleep() {
    // Read temperature
    sht3x_data = sht3x.readTemperatureAndHumidity(sht3x.eRepeatability_High);
    Temp = sht3x_data.TemperatureC;
    
    // Read voltage
    digitalWrite(MOSFET_PIN, HIGH);
    sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (analogRead(Battery_PIN) * VRef / 1023) * 2;
    }
    BatVolt = sum / n;
    digitalWrite(MOSFET_PIN, LOW);
        
    // Read next EEPROM addres used
    myEEPROM.get(32, EEPROM_Addres);
    
    // Write data to EEPROM
    myEEPROM.put(EEPROM_Addres, local_t);
    EEPROM_Addres += 32;
    while (myEEPROM.isBusy()) { delay(2); }
    myEEPROM.put(EEPROM_Addres, int(Temp * 100));
    EEPROM_Addres += 16;
    while (myEEPROM.isBusy()) { delay(2); }
    myEEPROM.put(EEPROM_Addres, int(BatVolt * 100));
    EEPROM_Addres += 16;
    while (myEEPROM.isBusy()) { delay(2); }
    
    // Update EEPROM registers
    NextLog = local_t + 43200;
    myEEPROM.put(0, NextLog);
    while (myEEPROM.isBusy()) { delay(2); }
    myEEPROM.put(32, EEPROM_Addres);
    while (myEEPROM.isBusy()) { delay(2); }
    
    // Turn OFF
    digitalWrite(Sleep_PIN, HIGH);
    delay(1000);
}