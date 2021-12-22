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
    // else continue to loop
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
    myEEPROM.get(4, EEPROM_Addres);
    
    // Write data to EEPROM
    myEEPROM.put(EEPROM_Addres, local_t);
    EEPROM_Addres += 4;
    while (myEEPROM.isBusy()) { delay(2); }
    myEEPROM.put(EEPROM_Addres, int(round(Temp * 10000)));
    EEPROM_Addres += 2;
    while (myEEPROM.isBusy()) { delay(2); }
    myEEPROM.put(EEPROM_Addres, int(round(BatVolt * 10000)));
    EEPROM_Addres += 2;
    while (myEEPROM.isBusy()) { delay(2); }
    
    // Update EEPROM registers
    local_t++; // Add one second to local time to prevent log  again at current time
    Calculate_NextLog();    
    myEEPROM.put(4, EEPROM_Addres);
    while (myEEPROM.isBusy()) { delay(2); }
    
    // Turn OFF
    digitalWrite(Sleep_PIN, HIGH);
    delay(1000);
}


////// Calculate next Log time
void Calculate_NextLog() {

    // Calclate days since UNIX Epoch, round done by casting result to int
    local_t_Days = int(local_t / 86400);
    if (local_t_Days % 2 == 0) {
        // Even day number, Night measurement is due
        EvenDay = true;
    }
    else {
        // Odd day number,  Day measurement is due
        EvenDay = false;
    }


    // Set system time to 3 h of present day
    // then calculate difference from current time
    setTime(3, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
    int t_to_3 = now() - local_t;
    
    
    // Set system time to 15 h of present day
    // then calculate difference from current time
    setTime(15, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
    int t_to_15 = now() - local_t;


    // Test differences
    if (EvenDay && t_to_3 > 0) {
        // Next log is 3 h this day
        setTime(3, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
        NextLog = now();
    }
    else if (EvenDay && t_to_3 < 0) {
        // Next log is 15 h next day
        setTime(15, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
        NextLog = now() + 86400;
    }
    else if (!EvenDay && t_to_15 > 0) {
        // Next log is 15 h this day
        setTime(15, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
        NextLog = now();
    }
    else {
        // Next log is 3 h next day
        setTime(3, 0, 0, RTCnow.day(), RTCnow.month(), RTCnow.year());
        NextLog = now() + 86400;
    }


    // Write Nextlog in EEPROM
    myEEPROM.put(0, NextLog);
    while (myEEPROM.isBusy()) { delay(2); }
}