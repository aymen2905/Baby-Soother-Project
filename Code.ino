#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <IRremote.h> // library for the PIR sensor

#define PIN_MOTOR       10 		 // Pin for the motor
#define PIN_NEOPIXELS   6 	 // Pin for the NeoPixels
#define NUMPIXELS       16          // number of neopixels
#define BUZZER_PIN      A2        // for the  piezo as 
#define SOUND_SENSOR    12      // Pin for  microphone
#define IR_RECEIVER_PIN 5    // Pin for SENSOR infr rouge
#define MOTOR_BUTTON    8

#define LIGHT_SENSOR    A1      // photoresistor
#define POTENTIOMETER   A0     // potentiometer to fix the sound

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);


#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
#define Microphone_Input 12


// change this to make the song slower or faster
int tempo = 140;

// change this to whichever pin you want to use
int buzzer = 3;

//Variable for Microphone Counter
unsigned long last_event = 0;


// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {
  // Happy Birthday

  NOTE_C4,4, NOTE_C4,8, 
  NOTE_D4,-4, NOTE_C4,-4, NOTE_F4,-4,
  NOTE_E4,-2, NOTE_C4,4, NOTE_C4,8, 
  NOTE_D4,-4, NOTE_C4,-4, NOTE_G4,-4,
  NOTE_F4,-2, NOTE_C4,4, NOTE_C4,8,

  NOTE_C5,-4, NOTE_A4,-4, NOTE_F4,-4, 
  NOTE_E4,-4, NOTE_D4,-4, NOTE_AS4,4, NOTE_AS4,8,
  NOTE_A4,-4, NOTE_F4,-4, NOTE_G4,-4,
  NOTE_F4,-2,
 
};
  
// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

// Variables FOR THE MULTITASKING
unsigned long previousMillisLum= 0;
unsigned long previousMillisMelody = 0;
unsigned long previousMillisMotor = 0;

const long intervalLum = 200; // Intervals of time to control the motor
const long intervalMelody = 200; // Intervals of time to play the melody
const long intervalMotor = 500; // Intervals of time for the motor
// State of the switch and the neopixels 
bool lastEvenMicro = LOW; // Store previous switch state
bool pixelsAllumes = false; // Indicator to know if the pixels are lit or not

void setup() {
  pixels.begin();
  pixels.setBrightness(100); //*BRAD* brightness control for the neopixels. 0 - 255.
  pixels.show();
  flash_NEOPIXELS();
  Serial.begin(9600); // Initializing serial communication for debugging
  
  
  //Inputs / Outputs
  pinMode(BUZZER_PIN, OUTPUT);// sound melody out
  pinMode(MOTOR_BUTTON, INPUT_PULLUP);// *BRAD* INPUT for the motor button.
  pinMode(PIN_MOTOR, OUTPUT); //DC MOTOR
  pinMode(SOUND_SENSOR, INPUT);// sound entrance 
  pinMode(LIGHT_SENSOR, INPUT_PULLUP);// lights
  pinMode(POTENTIOMETER, INPUT);// potentiométre
  pinMode(IR_RECEIVER_PIN, INPUT); // Configuration for the pin of the PIR sensor
  pinMode(BUZZER_PIN, OUTPUT);// sound melody out
  
  Serial.begin(9600); // Initializing serial communication for debugging
}

void loop() {
  /* Debugging motion sensor
  while (true){
    verifIR();
    delay(500);
  }
  */
  unsigned long currentMillis = millis();
  int interval =1000;
 
    // Read sensor values
  int soundValue = analogRead(SOUND_SENSOR);
    
  int lightValue = analogRead(LIGHT_SENSOR);
  int potentiometerValue = analogRead(POTENTIOMETER);
	int photoresistorValue = 0;
  
  CHECK_MOTOR(); //Determines if the motor should be switched on or not.
  analogWrite(PIN_MOTOR, 0); //Makes sure the motor is not
  // Multitasking
     if (verifIR()) //  to check if the baby is still there or not 
     {  // Task 1: Control brightness
       if (currentMillis - previousMillisLum >= intervalLum) 
			   {
            previousMillisLum = currentMillis;
	        photoresistorValue = analogRead(LIGHT_SENSOR);

       
	   if (photoresistorValue < 500) 
	   { //Dark
      colorWipe(pixels.Color(235, 83, 52), 50); // PINK //* BRAD * Sets the neopixels to pink so that there is some colour during the melody.
                  if (!Check_Microphone(currentMillis))// no sound and dark 
                      { doLightNeoPixels();
					  }
                      else
					  { // sound and dark
                        doSound();
                        doLightNeoPixelsClign();
                      }
	   
	   }
			   
       else
					  { // no dark 
                         if (!Check_Microphone(currentMillis))//no dak no sound 
                      {   analogWrite(PIN_MOTOR, 0); // Turn off the engine if the light is strong doLightNeoPixels()}
                          noLightNeoPixels();      // no light 
                      }
                             else {// no dark and sound 
                                doSound();
                                noLightNeoPixels(); // no light 
                             }
                            
                       
                             
                             } 
}

}
else
                             { analogWrite(PIN_MOTOR, 0); // Turn off the engine if the light is strong doLightNeoPixels()}
                               noLightNeoPixels();
                             }
                                          
}


                             
                             
                             
                             
                             
void doSound() {
  
// iterate over the notes of the melody.
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) 
  {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(BUZZER_PIN, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(BUZZER_PIN);
  }
}



void doLightNeoPixels() {
  colorWipe(pixels.Color(0, 0, 255), 50); // Bleu
  delay(500);
}

void doLightNeoPixelsClign() {
  colorWipe(pixels.Color(255, 0, 0), 50); // RED
  delay(500);
  colorWipe(pixels.Color(0, 255, 0), 50); // green
  delay(500);
  colorWipe(pixels.Color(0, 0, 255), 50); // Bleu
  delay(500);
}

void noLightNeoPixels() {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Turn off all NeoPixels
    pixels.show();
    delay(50);
  }
}

// Function to cycle color on NeoPixels
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(wait);
  }
}
// detected baby
bool verifIR() {
  int motionValue = digitalRead(IR_RECEIVER_PIN); // Read motion sensor value
  if (motionValue == 1) {
    //Serial.print("Motion Detected\n");
    return true; // Returns true if baby is detected
  } else {
    //Serial.print("Motion NOT Detected\n");
    return false; // Returns false if there is no baby 
  }
}


// Function to determine the state of the motor.
bool CHECK_MOTOR(){
  if (digitalRead(MOTOR_BUTTON == LOW)){
    while (digitalRead(MOTOR_BUTTON) == 0){
      //While the motor button is held down, run the motor.
      noLightNeoPixels();
      //Serial.print("MOTOR ON \n");
      analogWrite(PIN_MOTOR, 200);
      delay(500);

      //Debugging
      //Serial.print(digitalRead(MOTOR_BUTTON));
      //Serial.print("\n");
    }
  }
  else{
    return false; //returns false if the motor is not required.
  }
}



// - Function to flash the neopixels three times
 //- Useful upon startup so we know when the device is "enabled". */
void flash_NEOPIXELS(){
  for (int c = 0; c <3; c++){
    //Flash 3 times to indicate power.
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(100, 100, 100)); // Turn on all NeoPixels to white.
      pixels.show();
    }
    delay(1000); //one second delay
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Turn off all NeoPixels
      pixels.show();
    }
    delay(1000); //one second delay
  }
    
}
                         
//microphone function
bool Check_Microphone(unsigned long currentMillis){
  int evenMicro = digitalRead(SOUND_SENSOR);
  if (evenMicro == HIGH) {
    if (currentMillis - previousMillisMelody >= intervalMelody) {
    		previousMillisMelody = currentMillis;
            // Check if the microphone status has changed
  					if (evenMicro != lastEvenMicro) {
    						if (evenMicro == HIGH) {
      						// If the microphone is activated (switched to LOW), turn on the NeoPixels
      						return true;
    						} 						
                      		else 
							{
      						// Otherwise, turn off the NeoPixels
      						return false;
                              
    						}
    	
         }
    
    
    }
    lastEvenMicro = evenMicro; // Update previous state
  }
}