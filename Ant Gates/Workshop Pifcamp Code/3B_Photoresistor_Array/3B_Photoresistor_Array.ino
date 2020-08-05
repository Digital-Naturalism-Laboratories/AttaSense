//#include <MovingAverage.h>
#include "MovingAverage.h" // format for local files
/*
   Modulated PhotoGate

   basic photogate for an arduino where we pulse the light and measure the difference between on and off
   for constant re-calibration and better performance across varying ambient light levels

   Sensor pins are connected to one leg of a photo-resistor. This same leg is connected to a 1Kohm resistor. 
   The other leg of the 1Kohm resistor is connected to 5V on the Arduino

   the other leg of the photo resistor is connected to ground

   (This makes a basic voltage divider circuit)

   Nearby, you can attach LEDs to the LED pins

*/


int sensorPins[] = {A0, A1, A2, A3, A4};
#define totalSensors 5 // this number needs to match the number of entries above

int readingHIGH[totalSensors];
int readingLOW[totalSensors]; //Just measuring ambient light
int diffReading[totalSensors];
int avgReading[totalSensors];


int thresholds[] = {10, 10, 10, 10, 10}; //Need same number of thresholds as pins! can tweak individual thresholds
int detected[] = { -1, -1, -1, -1, -1}; //Need same number of thresholds as pins!

MovingAverage average[totalSensors](50);

int LEDPins[] = {12, 11, 10, 9, 8};
#define totalLEDs 5 // this number needs to match the number of entries above



void setup() {
  Serial.begin(57600);
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


  //First turn all the LEDs OFF
  for (int i = 0; i < totalLEDs; i++) {
    digitalWrite(LEDPins[i], LOW);   // turn the LED on (HIGH is the voltage level)
  }
  delay(10);//if using photo resistors you need some delay to let the sensors charge and discharge

  //measure all their values (with ambient light)
  for (int i = 0; i < totalSensors; i++) {

    readingLOW[i] = analogRead(sensorPins[i]);
  }


  //NEXT turn all the LEDs on
  for (int i = 0; i < totalLEDs; i++) {
    digitalWrite(LEDPins[i], HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  delay(1);//if using photo resistors you need some delay to let the sensors charge and discharge

  //measure all their values (with OUR light)
  for (int i = 0; i < totalSensors; i++) {

    readingHIGH[i] = analogRead(sensorPins[i]);

    //Do some calculations
    diffReading[i] = readingHIGH[i] - readingLOW[i];
    if (diffReading[i] < thresholds[i]) {
      detected[i] = 1; // We detected something in front of the gate! something is blocking the light!
    }
    else {
      detected[i] = 0; // nope the light value is normal
    }

    String title = "Sensor";
    
    String valueSplitter = ",";
    //Print out all the values
    //Serial.print(title+i+":"+diffReading[i]+valueSplitter);
    //Serial.print(readingHIGH[i]+valueSplitter);

    //Display the average reading
    average[i].update(diffReading[i]);
    //Serial.print(title + i + ":" + average[i].get() + valueSplitter);


    //Show the difference between the average, and the instantaneous new reading
    float instant = abs(average[i].get() - diffReading[i]);
    Serial.print(title + i + ":" + instant + valueSplitter);

  }


  Serial.println( );






}
