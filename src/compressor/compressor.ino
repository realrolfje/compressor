/*
 * Arduino based audio compressor
 */
const int inputPin = 3;
const int outputPin = 5;
const int clockoutpin = 6;
const int clippingledpin = 7;

const int attack = 1;
const int decay = 80;
const int targettop = 150;
const int clippinglevel = 500;

float oldtopfiltered = 0.0;
float newtopfiltered = 0.0;

void setup() {
}

void loop() {
  // Get sample and topdetector
  int sample = analogRead(inputPin);
  int topped = topDetect(sample);

  // Fast attack slow decay to topdetector
  oldtopfiltered = newtopfiltered;
  newtopfiltered = topFiltered(topped, oldtopfiltered);
  
  // Calculate gain from filtered top and target top
  float gain = calcgain(newtopfiltered);
  int output = round(gain * sample);
  analogWrite(outputPin, output);
  
  // Flip a pin to show our clockspeed
  digitalWrite(clockoutpin, !digitalRead(clockoutpin));
  
  // Turn on a led if the filtered top detector exceeds a limit
  digitalWrite(clippingledpin, newtopfiltered > clippinglevel);
}

int topDetect(int sample){
  return abs(512-sample);
}

float topFiltered(int top, int oldTop){
  int nrSamples = (top>oldTop) ? attack : decay;
  return float(oldTop + (top-oldTop)) / nrSamples;
}

float calcgain(float topFiltered){
  return float(targettop)/topFiltered;
}
