// Do not remove the include below
#include "ClosetController.h"
#include "PowerTail.h"
#include "Arduino.h"
#include "PIR.h"
#include "LowPower.h"

// Use pin 2 as wake up pin
const int wakeUpPin = 2;

PowerTail tail(A1, 12);
PIR pir(2, 13, 20, 0);

boolean action;

void wakeUp()
{
    // Just a handler for the pin interrupt.
	if (digitalRead(pir.pirPin) == HIGH) {
		tail.TurnOn();
		Serial.flush();
	} else {
		tail.TurnOff();
		Serial.flush();
	}
}

void setup() {
	Serial.begin(9600);
	pinMode(A1, OUTPUT);
	digitalWrite(A1, HIGH);
	Serial.print("calibrating sensor ");
	for (int i = 0; i < pir.calibrationTime; i++) {
		Serial.print(".");
		delay(1000);
	}
	Serial.println(" done");
	Serial.println("PIR sensor active!");

	Serial.println("starting up Power Tail!");
	delay(50);
	action = false;
}

void loop() {
	/*if (digitalRead(pir.pirPin) == HIGH && !action){
		Serial.println("On");
		action = true;
	}
	if (digitalRead(pir.pirPin) == LOW && action){
		Serial.println("Off");
		action = true;
	}*/
	// Do something here
	// Example: Read sensor, data logging, data transmission.
	pir.Motion();
	Serial.flush();
	// Allow wake up pin to trigger interrupt on low.
	attachInterrupt(0, wakeUp, CHANGE);

	// Enter power down state with ADC and BOD module disabled.
	// Wake up when wake up pin is low.
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

	// Disable external pin interrupt on wake up pin.
	detachInterrupt(0);
	Serial.flush();
}
