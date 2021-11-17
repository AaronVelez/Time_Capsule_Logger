void processSyncMessage() {
    unsigned long pctime;
    const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

    if (Serial.find(TIME_HEADER)) {
        pctime = Serial.parseInt();
        if (pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
            setTime(pctime); // Sync Arduino clock to the time received on the serial port
            rtc.adjust(DateTime(year(), month(), day(), hour(), minute(), second()));
        }
    }
}

time_t requestSync()
{
    Serial.write(TIME_REQUEST);
    return 0; // the time will be sent later in response to serial mesg
}
