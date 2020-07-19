#include <MovingAverage.h>

/*
   Modulated PhotoGate with LED as sensor

   basic photogate for an arduino where we pulse the light and measure the difference between on and off
   for constant re-calibration and better performance across varying ambient light levels

*/

//Create the Prototype of the function with handy defaults
int senseLED(int readpin, int chargepin, boolean measureChargeDifference = false, int timedropthreshold = 30, int senseDelay = 1);


int LEDPins[] = {12, 11, 12, 9, 8};
#define totalLEDs 2 // this number needs to match the number of entries above

int sensorPins[] = {A0, A1, A2, A3, A4};
int chargePins[]= {6,5,4,3,2};
#define totalSensors 2 // this number needs to match the number of entries above

int readingHIGH[totalSensors];
int readingLOW[totalSensors]; //Just measuring ambient light
int diffReading[totalSensors];
int avgReading[totalSensors];
int detected[totalSensors];

int thedelay = 1;
int microdelay = 1;

int thresholds[] = {20, 20, 20, 20, 20}; //Need same number of thresholds as pins! can tweak individual thresholds
//int [] = { -1, -1, -1, -1, -1}; //Need same number of thresholds as pins!

MovingAverage average[totalSensors](100);

//Optional Piezo option
int piezoHiLo[]={A2,A5};

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

  pinMode(piezoHiLo[0],OUTPUT);

  pinMode(piezoHiLo[1],OUTPUT);
  digitalWrite(piezoHiLo[0],HIGH);
  delay(200);
  digitalWrite(piezoHiLo[0],LOW);


}

void loop() {

  //First turn all the LEDs OFF
  for (int i = 0; i < totalLEDs; i++) {
    digitalWrite(LEDPins[i], LOW);   // turn the LED on (HIGH is the voltage level)
  }
  delay(thedelay);//if using photo resistors you need some delay to let the sensors charge and discharge
  //delayMicroseconds(1000); // for LED as sensor these delays need to be the same when on and off // LED as sensor doesn't seem to work well at times shorter than 1000 micros

  //measure all their values (with ambient light)
  for (int i = 0; i < totalSensors; i++) {
    readingLOW[i] = senseLED( sensorPins[i], chargePins[i]);
  }


  //NEXT turn all the LEDs on
  for (int i = 0; i < totalLEDs; i++) {
    digitalWrite(LEDPins[i], HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  delay(thedelay);//if using photo resistors you need some delay to let the sensors charge and discharge
  //delayMicroseconds(1000);

  //measure all their values (with OUR light)
  for (int i = 0; i < totalSensors; i++) {

    readingHIGH[i] =  senseLED( sensorPins[i], chargePins[i]);

    //Do some calculations
    diffReading[i] = readingHIGH[i] - readingLOW[i];


    String title = "Sensor";
    String valueSplitter = ",";
    //Print out all the values

    average[i].update(diffReading[i]);

    //Get raw reading
   //Serial.print(title + i + ":" + readingHIGH[i] + valueSplitter);
    //get difference reading
    //Serial.print(title + i + ":" + diffReading[i] + valueSplitter);

    //Display the average reading
    //Serial.print(title + i + ":"+ average[i].get() + valueSplitter);

    //Show the difference between the average, and the instantaneous new reading
    double instant = average[i].get() - diffReading[i];
     Serial.print(title + i + ":");
     Serial.print(instant);
     Serial.print(valueSplitter);


    if (instant > thresholds[i]) {
      detected[i] = 1; // We detected something in front of the gate! something is blocking the light!
       digitalWrite(piezoHiLo[0],HIGH);

    }
    else {
      detected[i] = 0; // nope the light value is normal
  digitalWrite(piezoHiLo[0],LOW);
    }
  }




  Serial.println( );

}

/*This function reads an LED as a photosensor using the reverse bias method
   it
   -discharges an LED rapidly (Turns it on via the charge pin)
   -Charges this led by turning it on in reverse
   -Discharges this LED and either
   ---Measures the time it takes for the LED's voltage to drop Low again (MeasureTimeUntilLow)
   ---Measures the voltage before discharge and at a set time after discharge (MeasureChargeDifference)

  Connect the readpin to an analog input pin, and the charge pin to whatever pin.
*/
int senseLED(int readpin, int chargepin, boolean measureChargeDifference, int timedropthreshold, int senseDelay) {


  // turn the LED on (HIGH is the voltage level)
  pinMode(chargepin, OUTPUT);
  digitalWrite(chargepin, HIGH);
  //Turn the sensor into an output
  pinMode(readpin, OUTPUT);
  digitalWrite(readpin, LOW);

  delayMicroseconds(senseDelay);

  //Reverse the bias and charge
  digitalWrite(chargepin, LOW);
  digitalWrite(readpin, HIGH);

  delayMicroseconds(senseDelay);


  if (timedropthreshold == false) {
    //Set readpin back to input and measure time to drain
    int startT = micros();
    pinMode(readpin, INPUT);
    // digitalWrite(A0, LOW);   // turn the LED on (HIGH is the voltage level)



    while (analogRead(readpin) > timedropthreshold) {
      //  if(10000<micros()-startT)break;
      // int reading= analogRead(readpin);
      //      Serial.println(reading);
    }
    int  totalTimeT = micros() - startT;

    delayMicroseconds(senseDelay);

    return totalTimeT;
  }
  else { // Use the method where we measure the drop of the voltage across a specified time.
    int firstR = analogRead(readpin);
    //Set readpin back to input and measure time to drain
    double startT = micros();
    pinMode(readpin, INPUT);

    delay(9); // delay for a standard amount each sample


    int secR = analogRead(readpin);
    int discharge = firstR - secR;
    return discharge;
  }

}




int readLEDBasic(int numReadings, int port) {            // Read analog value n times and avarage over those n times
  int total = 0;
  for (int x = 0; x < numReadings; x++) {
    total += analogRead(port);
    //delay(5);
  }
  return total / numReadings;
}
