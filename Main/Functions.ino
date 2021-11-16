
////// Take temperature and battery voltage data, store it in EEPROM, and put uHEX to sleep
void Meas_Rec_Sleep() {
    // Read temperature
    sht3x_data = sht3x.readTemperatureAndHumidity(sht3x.eRepeatability_High);
    Temp = sht3x_data.TemperatureC;
    // Read voltage
    BatVolt = (analogRead(Battery_PIN) * VRef / 1023) * 2;
    // Read last EEPROM addres used
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