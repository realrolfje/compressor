/*
 * Arduino based audio compressor
 */
const int inputPin = 3;
const int outputPin = 5; // 980Hz PWM
const int clockoutpin = 6;
const int clippingledpin = 7;

const int attack = 1;
const int decay = 80;
const int targettop = 150;
const float maxgain = 1.0 * (256/1024); // output is 8 bit, input is 10 bit.
const int clippinglevel = 500;

float oldtopfiltered = 0.0;
float newtopfiltered = 0.0;
boolean clockboolean = false;

void setup() {
  ringinit();
  pinMode(clockoutpin, OUTPUT);
  pinMode(clippingledpin, OUTPUT); 
}

void loop() {
  // Get sample and topdetector
  int sample = analogRead(inputPin);
  ringstore(sample);
  
  // Subtract DC offset and full-wave rectify
  int topped = abs(512-sample);

  // Create top detector filter with fast attack and slow
  // decay filter.
  oldtopfiltered = newtopfiltered;
  newtopfiltered = topFiltered(topped, oldtopfiltered);
  
  // Calculate gain from filtered top and target top
  float gain = min(maxgain, float(targettop)/newtopfiltered);
  
  // Apply calclated gain to a sample which is precisely the
  // number of samples ago as the attack time to prevent plopping.
  int output = round(gain * ringfetch());
  analogWrite(outputPin, output);
  
  // Flip a pin to show our clockspeed
  clockboolean = !clockboolean;
  digitalWrite(clockoutpin, clockboolean);
  
  // Turn on a led if the filtered top detector exceeds a limit
  digitalWrite(clippingledpin, newtopfiltered > clippinglevel);
}

float topFiltered(int top, int oldTop){
  int nrSamples = (top>oldTop) ? attack : decay;
  return float(oldTop + (top-oldTop)) / nrSamples;
}

