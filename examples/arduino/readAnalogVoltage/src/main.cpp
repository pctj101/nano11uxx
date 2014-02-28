/*
 ===============================================================================
 Name        : main.cpp
 Author      :
 Version     : v1.0.0
 Date		 :
 Copyright   :
 Description : for ARDUINO
 ===============================================================================
  	 	 	 	 	 	 	 	 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------

 ===============================================================================
 */
#include "uCXpresso.h"
#include "arduino.h"
/*===============================================================================*/
/*
  ReadAnalogVoltage
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  // Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 5: (nano11U37 P14)
  int sensorValue = analogRead(A5);
  // Convert the analog reading (which goes from 0 - 4095) to a voltage (0 - 3.3VV):
  float voltage = sensorValue * (3.3 / MAX_ADC_VALUE);
  // print out the value you read:
  Serial.println(voltage);
}
