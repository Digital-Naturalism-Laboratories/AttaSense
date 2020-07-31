#include <MovingAverage.h>

/*
   SMD LED Array Photo Imaging



*/

//Create the Prototype of the function with handy defaults
long senseLED(int readpin, int chargepin, boolean measureChargeDifference = false, int TimedropVoltagethreshold = 30, int senseDelay = 10, int chargeDiffTime = 9);

int sensorPins[] = {A0, A1, A2, A3, A4, A5}; // a standard Arduino Uno has 6 analog inputs, change these pins for whatever Analog inputs your microcontroller has
int chargePins[] = {12, 11, 10, 9, 8, 7};    //Pairs are made by the order they are listed. For instance A0 and 12 are a pair for one LED,
//Making the chargepin HIGH, and Sensepin LOW turns the LED on
//The SENSORPIN side of the LED is the CATHODE or NEGATIVE - side of the LED (Short leg of regular LED, marking side on SMD led)
//the CHARGEPIN side of an LED is the ANODE or POSITIVE + side of an LED (Long Leg of regular LED)

#define totalSensors 6 // this number needs to match the number of entries above or be less
//For a PURE led based sensing system, you need at least 2 sensors minimum

//Arrays to store readings we have captured
int readingLOW[totalSensors];  //Just measuring ambient light background reading of everything
int readingHIGH[totalSensors]; // Reading when the LED has illuminated the scene


int totaldiffReading[totalSensors]; //Total of the measurements between each light position sample and the background light

int avgReading[totalSensors];

MovingAverage average[totalSensors](100);

int detectionThresholds[] = {20, 20, 20, 20, 20, 20}; //Need same number of detectionThresholds as pins! can tweak individual detectionThresholds
int detected[totalSensors];                           // if the reading passes the detection threshold

//Delays for recharging sensors
int rechargeDelay = 1;
int microRechargeDelay = 100;

//Optional Piezo option
int piezoHiLo[] = {5, 2}; // First entry is the POSITIVE pin, second entry is our Pseudo ground pin)

void setup()
{
  Serial.begin(57600);

  //Initialize all LED sensors, flash through them for debugging
  int startupFlashSpeed = 100;
  for (int i = 0; i < totalSensors; i++)
  {
    pinMode(sensorPins[i], OUTPUT);
    digitalWrite(sensorPins[i], LOW);
  }
  for (int i = 0; i < totalSensors; i++)
  {
    pinMode(chargePins[i], OUTPUT);
    digitalWrite(chargePins[i], HIGH);
    delay(startupFlashSpeed);
  }

  for (int i = 0; i < totalSensors; i++)
  {
    pinMode(sensorPins[i], OUTPUT);
    digitalWrite(sensorPins[i], LOW);
  }
  for (int i = 0; i < totalSensors; i++)
  {
    pinMode(chargePins[i], OUTPUT);
    digitalWrite(chargePins[i], HIGH);
    delay(startupFlashSpeed);
    digitalWrite(chargePins[i], LOW);
  }


  //Initialize the pins for the Piezo output
  pinMode(piezoHiLo[0], OUTPUT);
  pinMode(piezoHiLo[1], OUTPUT);
  //give a test start up beep from the piezo
  digitalWrite(piezoHiLo[0], HIGH);
  delay(200);
  digitalWrite(piezoHiLo[0], LOW);
}

void loop()
{
  //First turn all the LEDs OFF
  //and measure all their values (with ambient light)
  for (int i = 0; i < totalSensors; i++)
  {
    pinMode(sensorPins[i], OUTPUT);
    digitalWrite(sensorPins[i], LOW);
    pinMode(chargePins[i], OUTPUT);
    digitalWrite(chargePins[i], LOW);
  }

  delay(rechargeDelay); //you often need some delay to let the sensors charge and discharge
  //delayMicroseconds(1000);

  //sense the ambient light
  for (int o = 0; o < totalSensors; o++)
  {
    readingLOW[o] = senseLED(sensorPins[o], chargePins[o]);
    //reset the difference readings
    totaldiffReading[o] = 0;
  }



  //Next
  // turn on all other  LEDs
  //and choose one LED to sense
  //
  for (int z = 0; z < totalSensors; z++)
  {
    //Go through all the other sensors and sense their impression of the current ON LED
    for (int i = 0; i < totalSensors; i++)
    {

      // turn ALL leds on
      pinMode(chargePins[i], OUTPUT);
      digitalWrite(chargePins[i], HIGH);
      //Turn the sensor into an output
      pinMode(sensorPins[i], OUTPUT);
      digitalWrite(sensorPins[i], LOW);
    }

    delay(rechargeDelay);

    //Check the latest readings on every sensor that is not the on light
    readingHIGH[z] = senseLED(sensorPins[z], chargePins[z]);
    //Add the latest difference to the total difference reading
    //use absolute value because we don't care if it makes it higher or lower, just that it is different from what it used to be
    totaldiffReading[z] = totaldiffReading[z] + abs(readingHIGH[z] - readingLOW[z]);


  }

  /*Calculations and Display
    //Now go back through all the LED calculations and display them after calculating
  */
  for (int i = 0; i < totalSensors; i++)
  {
    //update the average reading for each sensor
    average[i].update(totaldiffReading[i]);

    //Display all the calculations
    String title = "Sensor";
    String valueSplitter = ",";

    //Print out all the values

    //View the raw numReadings of the last sample it took
    Serial.print(title + i + ":" + readingHIGH[i] + valueSplitter);

    //get total difference reading
    //Serial.print(title + i + ":" + totaldiffReading[i] + valueSplitter);

    //Display the average reading
    //Serial.print(title + i + ":"+ average[i].get() + valueSplitter);

    //Show the difference between the average, and the instantaneous new reading
    double instant = average[i].get() - totaldiffReading[i];
    // Serial.print(title + i + ":");    Serial.print(instant);    Serial.print(valueSplitter);




    if (instant > detectionThresholds[i])
    {
      detected[i] = 1; // We detected something in front of the gate! something is blocking the light!
      digitalWrite(piezoHiLo[0], HIGH);
    }
    else
    {
      detected[i] = 0; // nope the light value is normal
      digitalWrite(piezoHiLo[0], LOW);
    }
  }

  Serial.println();
}

/*This function reads an LED as a photosensor using the reverse bias (Forrest Mims) method
   it
   -discharges an LED rapidly (Turns it on via the charge pin)
   -Charges this led by turning it on in reverse
   -Discharges this LED and either
   ---Measures the time it takes for the LED's voltage to drop Low again (MeasureTimeUntilLow)
   ---Measures the voltage before discharge and at a set time after discharge (MeasureChargeDifference)

  Connect the readpin to an analog input pin, and the charge pin to whatever pin.
*/
long senseLED(int readpin, int chargepin, boolean measureChargeDifference, int TimedropVoltagethreshold, int senseDelay, int chargeDiffTime)
{

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

  if (measureChargeDifference == false)
  {
    //Set readpin back to input and measure time to drain
    int val = 10000;
    int startT = micros();
    pinMode(readpin, INPUT);
 
    while (val > TimedropVoltagethreshold )
    {
      val = analogRead(readpin);
      if (10000000.0 < micros() - startT) {
        val = 0;
      }
      // int reading= analogRead(readpin);
      //      Serial.println(reading);
    }
    long totalTimeT = micros() - startT;

    delayMicroseconds(senseDelay);

    return totalTimeT;
  }
  else
  { // Use the method where we measure the drop of the voltage across a specified time.
    int firstR = analogRead(readpin);
    //Set readpin back to input and measure time to drain
    double startT = micros();
    pinMode(readpin, INPUT);

    delay(chargeDiffTime); // delay for a standard amount each sample

    int secR = analogRead(readpin);
    long discharge = firstR - secR;
    return discharge;
  }
}

//Old method, you basically just plug it into an Analog Port and sense the voltage change
int readLEDBasic(int numReadings, int port)
{ // Read analog value n times and avarage over those n times
  long total = 0;
  for (int x = 0; x < numReadings; x++)
  {
    total += analogRead(port);
    //delay(5);
  }
  return total / numReadings;
}
