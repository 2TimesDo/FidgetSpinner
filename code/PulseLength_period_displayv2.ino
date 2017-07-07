#include <U8g2lib.h>
#include <U8x8lib.h>

const int intPULSES=2;      //Number of Pulses to get period for per read cycle - NTOE doesn't average so only works for 2 currently.
const int MSFLASHFAST=250;              //mS to flash LED fast
int pinIRSensor = 2,pinLED=4;           //Digital pin 2 has a IRSensor attached to it, pin 4 has an LED
bool bolLEDState=false;                 //Current LED State
volatile long lngPulseTime[intPULSES];  //Array of time at interrupt
volatile int intPulseCount=0;           //Count of how many pulses we've read


//Setup for the OLED display we're using
//U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI(rotation, clock, data (MOSI), cs, dc [, reset]);
U8G2_SSD1306_128X64_VCOMH0_2_4W_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8 );

// the setup routine runs once when you press reset:
void setup() {
  //Make the IRSensor's pin an input with a pullup resistor
  pinMode(pinIRSensor, INPUT);
  digitalWrite(pinIRSensor, HIGH);
  //Setup a normal LED
  pinMode(pinLED,OUTPUT);
  //Init the OLED display
  u8g2.begin();
}

void ISRPulse(){
  //Called whenever a new pulse arrives. 
  lngPulseTime[intPulseCount]=micros();   //Save the current value of the micro second (uS) counter
  intPulseCount++;                        //Add 1 to the index into the PulseTime array
}

void loop() {
  long  lngPerioduS             //How long in uS between pulses
  int   intRPS;                 //Revs per Second
  int   intSpinnerLegs=3;       //number of legs on a spinner - would be nice to make this a user input 
  long  lngFlash=0, lngStop;    //LED Flash counter, and stop time counter
  bool  bolWait,bolRunning;     //Are we waiting for a spinner, Do we still have a spinner
  int   intMaxRPS;              //The maximum REP we've seen for this spinner
  long  lngTotal;               //The total spin time

  //Display 'Waiting' message
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(30,40,"waiting");
  } while ( u8g2.nextPage() );

  //Do nothing till we see a spinner
  while (digitalRead(pinIRSensor)==HIGH){}

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
    lngStop=millis()+3000;          //If we don't see a spinner leg by this time, run has stoped
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

      //Display Current RPS
      u8g2.firstPage();
      do {
        u8g2.setFont(u8g2_font_freedoomr25_tn);
        if (intRPS<10){
        u8g2.setCursor(55,40);}
        else{
          u8g2.setCursor(42,40);
        }
        u8g2.print(intRPS);
      } while ( u8g2.nextPage() );
       
    }else{      //Timed out
      lngTotal = (millis()-lngTotal)/1000;  //Calculate total run time and display it and total time
      u8g2.firstPage();
      do {
        u8g2.drawFrame(0,0,128,63);
        u8g2.setCursor(15,20);
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.print("max rps = ");
        u8g2.setFont(u8g2_font_ncenB12_tr);
        u8g2.print(intMaxRPS);
        u8g2.setCursor(12,44);
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.print("spin time = ");
        u8g2.setFont(u8g2_font_ncenB12_tr);
        u8g2.print(lngTotal);
      } while ( u8g2.nextPage() );
      delay(10000);   //Wait so user can see the totals
    }
  }
  while (digitalRead(pinIRSensor)==LOW){}     //Wait till we see a spinner.

}



