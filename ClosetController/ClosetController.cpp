// Do not remove the include below
#include "ClosetController.h"
#include "PowerTail.h"
#include "Arduino.h"
#include "PIR.h"


PowerTail tail(A1, 12);
PIR pir(7, 13, 20, 0);

boolean action;

void setup() {
	Serial.begin(9600);
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

	pir.Motion();
	if (pir.presence && !action) {
		tail.ManualPower();
		action = true;
	} else if (!pir.presence && action) {
		tail.ManualPower();
		action = false;
	}
}
