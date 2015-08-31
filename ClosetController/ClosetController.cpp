// Do not remove the include below
#include "ClosetController.h"
#include "PowerTail.h"
#include "Arduino.h"
#include "PIR.h"

PowerTail tail(A1, 12);
PIR pir(7, 13, 30);

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
	pir.Motion();
	/*if (pir.presence == true && action != action) {
		tail.ManualPower();
		action = !action;
	} else if (pir.presence == true && tail.powerOn == true) {
		//Serial.println("Still sensing presence");
	} else {
		tail.ManualPower();
		action = !action;
	}*/
	if (pir.presence && action == false) {
		tail.ManualPower();
		action = true;
	} else if (!pir.presence && action == true) {
		tail.ManualPower();
		action = false;
	}

}
