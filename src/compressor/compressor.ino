/*
 * Arduino based audio compressor
 */
const int inputPin = 3;
const int outputPin = 5; // 980Hz PWM
const int clockoutpin = 6;
const int clippingledpin = 7;

const int attack = 1;
const int decay = 80;
const float targettop = 150.0;
const float maxgain = 1.0 * (256/1024); // output is 8 bit, input is 10 bit.
const int clippinglevel = 500;

// Ringbuffer
const int buffersize = attack;
int ring[buffersize];

void setup() {
  // Initialize ring buffer
  for (int i=0; i<buffersize; i++) {
    ring[i]=0;
  }

  changePrescaler();
  pinMode(clockoutpin, OUTPUT);
  pinMode(clippingledpin, OUTPUT); 
}

void loop() {
  // Initializing variables inside the function
  // increases memory access performance.
  float oldtopfiltered = 0.0;
  float newtopfiltered = 0.0;
  boolean clippingboolean = false;
  int ringpointer=0;
  
  // While(true) prevents the code from leaving the loop()
  // function and re-entering it, with all the stack manipulation
  // that comes with it.
  while(true) {
    // Set clockoutputpin high, to signal the start of the loop.
    // Outputpin should oscilate at 14.6 kHz
    digitalWrite(clockoutpin, HIGH);

    // Get audio sample
    int sample = analogRead(inputPin);
    
    // Set clockoutpin low, to signal we're done with taking
    // an audio sample. Taking the audio sample takes between
    // 20 uS and 30uS, about 33% of the loop time.
    digitalWrite(clockoutpin, LOW);

    // Store sample in the ringbuffer
    ringpointer = (ringpointer+1) % buffersize;
    ring[ringpointer] = sample;
    
    // Subtract DC offset and full-wave rectify
    int topped = abs(512-sample);
  
    // Create top detector filter with fast attack and slow
    // decay filter.
    oldtopfiltered = newtopfiltered;
    newtopfiltered = 0;
//    topFiltered(topped, oldtopfiltered);
    
    // Calculate gain from filtered top and target top
    float gain = min(maxgain, targettop/newtopfiltered);
    
    // Apply calclated gain to a sample which is precisely the
    // number of samples ago as the attack time to prevent plopping.
    int tailpointer = (ringpointer + buffersize - 1) % buffersize;
    int output = round(gain * ring[tailpointer]);
    analogWrite(outputPin, output);
    
    
    // Turn on a led if the filtered top detector exceeds a limit
    if (clippingboolean != (newtopfiltered > clippinglevel)) {
      clippingboolean = (newtopfiltered > clippinglevel);
      digitalWrite(clippingledpin, clippingboolean);
    }
  }
}

// Fast attack, slow decay filter.
float topFiltered(int top, int oldTop){
  int nrSamples = (top>oldTop) ? attack : decay;
  return float(oldTop + (top-oldTop)) / nrSamples;
}

// Speeds up A/D Conversion.
// With the default prescaler settings, the Arduino samples at about 8KHz.
// With these prescaler settings, the Arduino sampels at about 21KHz.
// From http://forum.arduino.cc/index.php?topic=6549.0
void changePrescaler(){
  // defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// set prescale to 16
sbi(ADCSRA,ADPS2) ;
cbi(ADCSRA,ADPS1) ;
cbi(ADCSRA,ADPS0) ;
}
