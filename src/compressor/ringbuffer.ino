/*
 * Simple ringbuffer
 */
const int buffersize = attack;

int ring[buffersize];
int ringpointer=0;

void ringinit() {
  for (int i=0; i<buffersize; i++) {
    ring[i]=0;
  }
}

void ringstore(int val) {
  int ringpointer = (ringpointer+1) % buffersize;
  ring[ringpointer] = val;
}

int ringfetch() {
  int tailpointer = (ringpointer + buffersize - 1) % buffersize;
  return ring[tailpointer];
}
