/*
  Matt Joyce
  Laundry Alert Timer
  Runs on CC3220SF Launchpad board with a SW-420 Vibration Sensor, two LEDs,
  and two 470 Ohm resistors on a solderless breadboard.

  Turns on red led when the SW-420 Vibration sensor detects movement above 
  a given threshold (VIBRATION_SENSITIVITY).

  vib_flag is initially 0, accounting for the long period without
  vibration from the beginning of the wash cycle to the first spin cycle.
  When the first vibration is detected at the first spin cycle, the Red LED
  turns on and vib_flag is set to 1. 

  Once vib_flag is set, if MAX_WAIT has been exceeded, call send_sms() function
  to alert user that laundry cycle is complete via text message. Currently, the 
  TM4C123 board is standing in for the CC3220SF and the blink_green() function 
  is standing in for the SMS alert while I'm working out an incompatibility issue
  with the SMS API. 
   
  Two 470 Ohm resistors are used to limit current on LEDs. 
  The red led is at PB_1, the green led is at PB_2 and the SW-420 is at PB_0.

  References:
  1) https://www.youtube.com/watch?v=2pc2ehkSEAI&t=134s for assistance 
  initializing vibration sensor.
  2) Discussions with Eric Vogel for the initial project idea.
  
 */

#define MAX_WAIT 330000
#define BLINK_DELAY 1000
#define MAIN_LOOP_DELAY 25
#define BAUD_RATE 112500
#define VIBRATION_SENSITIVITY 25

const int VIB_SENSOR = PB_0;     
const int R_LED =  PB_1; 
const int G_LED = PB_2;     

unsigned long measurement = 0;  
unsigned long now = 0; 
unsigned long time_since_vib = 0; 
unsigned long vib_flag = 0; 

// Initialization function in Energia environment.
// Initialize LED pins to output and Vibration Sensor to Input.
// Initialize baud rate to BAUD_RATE.
void setup() {
  // initialize the LED pins as an output:
  pinMode(R_LED, OUTPUT);    
  pinMode(G_LED, OUTPUT);  
  pinMode(VIB_SENSOR, INPUT);
  Serial.begin(BAUD_RATE);      
}

// Main loop checks the vibration reading of the SW-420 vibration
// sensor. Waits for initial vibration to set vib_flag. Thereafter, if 
// vibration is detected, the red led turns on. If no vibration is 
// detected, the internal clock increments and tests against MAX_WAIT.
// If the time exceeds MAX_Wait, blink the green led, signifying the end of 
// the wash/dry cycle. 
void loop(){
  // read the measurement value.
  measurement = vibration(); 

  // Print measurement value to console.
  delay(MAIN_LOOP_DELAY);  

  // Washing machine has already reached first spin cycle. 
  // Begin timing of MAX_WAIT
  if(vib_flag == 1){
    
    // Vibration detected, turn red led on.
    // Mark start of time since last detected vibration.
    if(measurement > VIBRATION_SENSITIVITY){
      digitalWrite(R_LED, HIGH);
      now = millis(); 
    }

    // No vibration detected. Turn off Red LED.
    // Measure time elapsed since vibration, compare
    // against MAX_WAIT. If MAX_WAIT has been exceeded, 
    // blink green led. 
    else {
      digitalWrite(R_LED, LOW); 
      time_since_vib = (millis() - now);

      if (time_since_vib > MAX_WAIT){
        blink_green();
      }
    }
  }

  // There hasn't been any vibration yet (washing machine has
  // not reached first spin cycle.
  else{
    if(measurement > VIBRATION_SENSITIVITY){
      digitalWrite(R_LED, HIGH);
      vib_flag = 1; 
    }
    else{
      digitalWrite(R_LED, LOW);
    }
  }
}

// Measure and return current vibration level.
long vibration(){
  long measurement = pulseIn(VIB_SENSOR, HIGH);
  return measurement; 
}

// Blink green led at one second intervals. 
void blink_green(){
  digitalWrite(G_LED, HIGH);
  delay(BLINK_DELAY);
  digitalWrite(G_LED, LOW);  
  delay(BLINK_DELAY);
}
