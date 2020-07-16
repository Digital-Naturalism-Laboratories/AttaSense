/*
   Modulated PhotoGate

   basic photogate for an arduino where we pulse the light and measure the difference between on and off
   for constant re-calibration and better performance across varying ambient light levels

*/


int sensorPins[] = {A0, A1, A2, A3, A4};
#define totalSensors = 5; // this number needs to match the number of entries above

int rawReadings[totalSensors];
int readingHIGH[totalSensors];
int readingLOW[totalSensors];



int LEDPins[] = {12, 11, 10, 9, 8};
#define totalLEDs = 5; // this number needs to match the number of entries above



void setup() {

  //Initialize all sensors connected
  for (int i = 0; i < totalSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  //initialize outputs (delays and turn ons for debugging check
  for (int i = 0; i < totalLEDs; i++) {
    pinMode(LEDPins[i], OUTPUT);
    digitalWrite(LEDPins[i], HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(40);                       // wait for a second
    digitalWrite(LEDPins[i], LOW);    // turn the LED off by making the voltage LOW
    delay(20);

  }


}

void loop() {

}
