#include <MovingAverage.h>

/*
   SMD LED Array Photo Imaging



*/

//Create the Prototype of the function with handy defaults
int senseLED(int readpin, int chargepin, boolean measureChargeDifference = false, int timedropthreshold = 30, int senseDelay = 1);



int sensorPins[] = {A0, A1, A2, A3, A4, A5};
int chargePins[] = {12, 11, 10, 9, 8}; //Pairs are made by the order they are listed. For instance A0 and 12 are a pair for one LED,
//Making the chargepin HIGH, and Sensepin LOW turns the LED on
#define totalSensors 4 // this number needs to match the number of entries above or be less

int readingLOW[totalSensors]; //Just measuring ambient light background reading of everything

int readingHIGH[totalSensors];// Reading when the LED has illuminated the scene
int totaldiffReading[totalSensors]; //Total of the measurements between each sample and the background light
int avgReading[totalSensors];
int detected[totalSensors];

int thedelay = 1;
int microdelay = 1;

int thresholds[] = {20, 20, 20, 20, 20}; //Need same number of thresholds as pins! can tweak individual thresholds
//int [] = { -1, -1, -1, -1, -1}; //Need same number of thresholds as pins!

MovingAverage average[totalSensors](100);

//Optional Piezo option
int piezoHiLo[] = {6, 7};

void setup() {
  Serial.begin(57600);
  //Initialize all sensors connected
  for (int i = 0; i < totalSensors; i++) {
    pinMode(sensorPins[i], INPUT);
  }



  pinMode(piezoHiLo[0], OUTPUT);

  pinMode(piezoHiLo[1], OUTPUT);
  digitalWrite(piezoHiLo[0], HIGH);
  delay(200);
  digitalWrite(piezoHiLo[0], LOW);


}

void loop() {
  //First turn all the LEDs OFF
  //And Sense every single LED
  delay(thedelay);//if using photo resistors you need some delay to let the sensors charge and discharge
  //delayMicroseconds(1000); // for LED as sensor these delays need to be the same when on and off // LED as sensor doesn't seem to work well at times shorter than 1000 micros

  //measure all their values (with ambient light)
  for (int i = 0; i < totalSensors; i++) {
    readingLOW[i] = senseLED( sensorPins[i], chargePins[i]);
    //reset the difference readings
    totaldiffReading[i] = 0;
  }


  //Next Turn one LED on permanently
  //And sense every other Led's impression of that one

  delay(thedelay);//if using photo resistors you need some delay to let the sensors charge and discharge
  //delayMicroseconds(1000);

  for (int z = 0; z < totalSensors; z++) {

    // turn the chosen LED on
    pinMode(chargePins[z], OUTPUT);
    digitalWrite(chargePins[z], HIGH);
    //Turn the sensor into an output
    pinMode(sensorPins[z], OUTPUT);
    digitalWrite(sensorPins[z], LOW);

    for (int i = 0; i < totalSensors; i++) {
      if (z = i) {
        break; //break out of this sensor loop if this led is the ON led
      }
      //Check the latest readings on every sensor that is not the light
      readingHIGH[i] =  senseLED( sensorPins[i], chargePins[i]);

      //Add the latest difference to the total difference reading
      //use absolute value because we don't care if it makes it higher or lower, just that it is different from what it used to be
      totaldiffReading[i] = totaldiffReading[i] + abs(readingHIGH[i] - readingLOW[i]);
    }
  }

  //Now go back through all the LED calculations and display them after calculating

  for (int i = 0; i < totalSensors; i++) {

    //Display all the calculations
    String title = "Sensor";
    String valueSplitter = ",";
    //Print out all the values

    average[i].update(totaldiffReading[i]);

    //Get raw reading
    //Serial.print(title + i + ":" + readingHIGH[i] + valueSplitter);
    //get difference reading
    //Serial.print(title + i + ":" + diffReading[i] + valueSplitter);

    //Display the average reading
    //Serial.print(title + i + ":"+ average[i].get() + valueSplitter);

    //Show the difference between the average, and the instantaneous new reading
    double instant = average[i].get() - totaldiffReading[i];
    Serial.print(title + i + ":");
    Serial.print(instant);
    Serial.print(valueSplitter);


    if (instant > thresholds[i]) {
      detected[i] = 1; // We detected something in front of the gate! something is blocking the light!
      digitalWrite(piezoHiLo[0], HIGH);

    }
    else {
      detected[i] = 0; // nope the light value is normal
      digitalWrite(piezoHiLo[0], LOW);
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
