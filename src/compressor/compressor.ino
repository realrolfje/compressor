/*
 * Arduino based audio compressor
 */
const int inputPin = 3;
const int outputPin = 5; // 980Hz PWM
const int clockoutpin = 6;
const int clippingledpin = 7;

const int attack = 1;
const int decay = 1024;
const byte shiftbits = 5;
const float targettop = 256 * (2^shiftbits);

// Minimum detected top (this prevents
// the gain from going up unbridled when
// there is (almost) not input.
const float mintop = 32 * (2^shiftbits);

// Clipping level is measured on shifted
// and filtered top detector. Unshifted
// absolute max level is 1024/2 = 512.
// some bits room is nice to light the led
// before really clipping.
const int clippinglevel = 506 << shiftbits;

// Ringbuffer
const int buffersize = attack+1;
int ring[buffersize];

void setup() {
  setPwmFrequency(outputPin, 1);

  // Initialize ring buffer
  for (int i=0; i<buffersize; i++) {
    ring[i]=0;
  }

//  changePrescaler();
  pinMode(clockoutpin, OUTPUT);
  pinMode(clippingledpin, OUTPUT); 
  
}

void loop() {
  // Initializing variables inside the function
  // increases memory access performance.
  int oldtopfiltered = 0;
  int newtopfiltered = 0;
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
    // Bit shhift 6 places to the left so we don't have to
    // calculate with floats, and remove the DC offset.
    int sample = ((analogRead(inputPin) - 512) * (2^shiftbits));
    
    // Set clockoutpin low, to signal we're done with taking
    // an audio sample. Taking the audio sample takes between
    // 20 uS and 30uS, about 33% of the loop time.
    digitalWrite(clockoutpin, LOW);
    
    // Store sample in the ringbuffer
    ringpointer = (ringpointer + 1) % buffersize;
    ring[ringpointer] = sample;
    
    // Full wave rectify top detector
    int topped = abs(sample);

    // Fast attack slow decay top detector calculation
    oldtopfiltered = newtopfiltered;
    int nrSamples = (topped > oldtopfiltered) ? attack : decay;
    newtopfiltered = max(mintop, oldtopfiltered + ((topped-oldtopfiltered) / nrSamples));

    // Apply calclated gain to a sample which is precisely the
    // number of samples ago as the attack time to prevent plopping.
    int tailpointer = (ringpointer + 1) % buffersize;
    
    // Apply gain based on the top detector on the oldest sample in the buffer
    int output = round((targettop/newtopfiltered) * ring[tailpointer]);
    
    // Add dc offset and reduce to 8 bit.
    output = ((output / (2^shiftbits)) + 512) / 4 ;
    analogWrite(outputPin, output);
  }
//  
//    
//
//    // Store sample in the ringbuffer
//    ringpointer = (ringpointer + 1) % buffersize;
//    ring[ringpointer] = sample;
//    
//    // Subtract DC offset and full-wave rectify
//    int topped = abs(sample);
//  
//    // Create top detector filter with fast attack and slow
//    // decay filter.
//    oldtopfiltered = newtopfiltered;
//    int nrSamples = (topped > oldtopfiltered) ? attack : decay;
//    newtopfiltered = max(mintop, oldtopfiltered + ((topped-oldtopfiltered) / nrSamples));
//    
//    // Apply calclated gain to a sample which is precisely the
//    // number of samples ago as the attack time to prevent plopping.
//    int tailpointer = (ringpointer + buffersize - 1) % buffersize;
//    
//    // Calculate gain from filtered top and target top, and correct for
//    // the fact that the input was 10 bits and the output is 8 buts.
//    int output = (((targettop * ring[tailpointer]) / newtopfiltered) + 0x7fff) >> (shiftbits+1);
//    
//    analogWrite(outputPin, output);
//    
//    // Turn on a led if the filtered top detector exceeds a limit
//    if (clippingboolean != (newtopfiltered > clippinglevel)) {
//      clippingboolean = (newtopfiltered > clippinglevel);
//      digitalWrite(clippingledpin, clippingboolean);
//    }
//  }
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

/**
 * Divides a given PWM pin frequency by a divisor.
 *
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 *
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 *
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 *
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
