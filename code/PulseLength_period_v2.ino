/*
This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
(http://creativecommons.org/licenses/by-sa/4.0/)
Based on an original piece by 2.Times Do CIC at https://github.com/2timesdo/FidgetSpinner
 */

const int intPULSES=2;                  //Number of Pulses to get period for per read cycle - NTOE doesn't average so only works for 2 currently.
const int MSFLASHFAST=250;              //mS to flash LED fast
const int MSTIMEOUT=3000;               //mS to wait to see a spinner leg pass when we're running
const int MSPRINT=5000;                 //mS to output when running
int pinIRSensor = 2,pinLED=4;           //Digital pin 2 has a IRSensor attached to it, pin 4 has an LED
bool bolLEDState=false;                 //Current LED State
volatile long lngPulseTime[intPULSES];  //Array of time at interrupt
volatile int intPulseCount=0;           //Count of how many pulses we've read


// the setup routine runs once when you press reset:
void setup() {  //Make the IRSensor's pin an input with a pullup resistor
  pinMode(pinIRSensor, INPUT);
  digitalWrite(pinIRSensor, HIGH);
  //Setup a normal LED
  pinMode(pinLED,OUTPUT);

  //Init serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial.println("Starting...");
}

void ISRPulse(){
  //Called whenever a new pulse arrives. 
  lngPulseTime[intPulseCount]=micros();   //Save the current value of the micro second (uS) counter
  intPulseCount++;                        //Add 1 to the index into the PulseTime array
}

void loop() {
  long  lngPerioduS;            //How long in uS between pulses
  int   intRPS;                 //Revs per Second
  int   intSpinnerLegs=3;       //number of legs on a spinner - would be nice to make this a user input 
  long  lngFlash=0, lngPrint, lngStop;    //LED Flash counter, Print counter and stop time counter
  bool  bolWait,bolRunning;     //Are we waiting for a spinner, Do we still have a spinner
  int   intMaxRPS;              //The maximum REP we've seen for this spinner
  long  lngTotal;               //The total spin time

  //Display 'Waiting' message
  Serial.println("Waiting...");

  //Do nothing till we see a spinner
  while (digitalRead(pinIRSensor)==HIGH){}

  //Tell user we're running
  Serial.println("Running...");
  //Zero run variables and enter the running loop
  bolRunning = true;
  intMaxRPS = 0;
  lngTotal = millis();
  while (bolRunning){               //Loop round till we haven't seen a spinner leg go past
    //Zero the PulseTimes array
    for (int i=0;i<intPULSES;i++)
      lngPulseTime[i]=0;
    intPulseCount=0;                //Reset the number of pulses we've seen
    lngFlash=millis()+MSFLASHFAST;  //Next time to flash the LED
    lngPrint=millis()+MSPRINT;      //Next time to print the current figures
    lngStop=millis()+MSTIMEOUT;     //If we don't see a spinner leg by this time, run has stoped
    attachInterrupt(digitalPinToInterrupt(pinIRSensor), ISRPulse, FALLING); //Interrupt to the ISRPulse Funciton every falling edge on pinIRSensor
    while ((intPulseCount<=1)&&bolRunning){ //Loop until we've seen 2 pulses or timed out.
      if (millis()>lngStop){        //Has the clock gone beyond the stop time
        bolRunning=false;
      }
      //LED flash
      if (lngFlash<millis()){
        bolLEDState=not bolLEDState;
        digitalWrite(pinLED,bolLEDState); 
        lngFlash=millis()+MSFLASHFAST;
      }
    }
    detachInterrupt(digitalPinToInterrupt(pinIRSensor));    //Stop interrupting
    if (bolRunning){
      lngPerioduS=lngPulseTime[1]-lngPulseTime[0];  //Period in uS
      intRPS=(1000000/lngPerioduS)/intSpinnerLegs;  //Revs Per Second

      if (intRPS>intMaxRPS){    //Update max RPS if required
          intMaxRPS = intRPS;
      }

      //If MSPRINT time has passed, display Current RPS
      if (lngPrint<millis()){
        lngPrint=millis()+MSPRINT;      //Next time to print the current figures
        Serial.println(intRPS);
      }
       
    }else{      //Timed out
      lngTotal = (millis()-lngTotal)/1000;  //Calculate total run time and display it and total time
      Serial.print("max rps = ");
      Serial.println(intMaxRPS);
      Serial.print("spin time = ");
      Serial.println(lngTotal);
    }
  }
  //Display 'Waiting' message
  Serial.println("Waiting...");
  while (digitalRead(pinIRSensor)==LOW){}     //Wait till we see a spinner.
}



