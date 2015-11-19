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

float oldtopfiltered = 0.0;
float newtopfiltered = 0.0;
boolean clockboolean = false;
boolean clippingboolean = false;

void setup() {
  ringinit();
  changePrescaler();
  pinMode(clockoutpin, OUTPUT);
  pinMode(clippingledpin, OUTPUT); 
}

void loop() {
  // While(true) prevents the code from leaving the loop()
  // function and re-entering it, with all the stack manipulation
  // that comes with it.
  while(true) {
    // Get sample and topdetector
    int sample = analogRead(inputPin);
    ringstore(sample);
    
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
    int output = round(gain * ringfetch());
    analogWrite(outputPin, output);
    
    // Flip a pin to show our clockspeed
    clockboolean = !clockboolean;
    digitalWrite(clockoutpin, clockboolean);
    
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
