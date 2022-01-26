/*
 Name:		Test_Battery_Voltage.ino
 Created:	1/24/2022 10:40:53 PM
 Author:	aivel
*/

int Battery_PIN = A3;
int MOSFET_PIN = 2;

const float VRef = 3.3;

float BatVolt = -1;     // Battery voltage
const int n = 10;      // measure n times the ADC input for averaging
float sum = 0;          // shift register to hold ADC data

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);

    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);

}

// the loop function runs over and over again until power down or reset
void loop() {
    // Read voltage
    digitalWrite(MOSFET_PIN, HIGH);
    
    sum = 0;
    for (int i = 0; i < n; i++) {
        delay(100);
        int RawBits = analogRead(Battery_PIN);
        Serial.print(F("Analog raw: "));
        Serial.println(RawBits);

        sum += (analogRead(Battery_PIN) * VRef / 1023) * 2;
    }
    BatVolt = sum / n;
    digitalWrite(MOSFET_PIN, LOW);


    

    Serial.print(F("Battery Voltage: "));
    Serial.println(BatVolt, 4);

    delay(1000);

  
}
