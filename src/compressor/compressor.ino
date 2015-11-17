/*
 * Arduino based audio compressor
 */


const int inputPin = 3;
const int outputPin = 5;

const int buffersize = 100;
int samplebuffer[buffersize];
int headpointer=0;
int tailpointer=buffersize-1;

void setup() {
  // Init buffer
  for (int i=0; i < buffersize; i++){
    samplebuffer[i]=0;
  } 
}

void loop() {
  samplebuffer[headpointer] = analogRead(inputPin);
  analogWrite(outputPin, samplebuffer[tailpointer]);

  headpointer = (headpointer++) % buffersize;
  tailpointer = (tailpointer++) % buffersize;
}
