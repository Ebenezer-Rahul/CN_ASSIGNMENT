#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.

typedef struct Packet {
  int type;
  int sequenceNumber;
  char payload[1024];
  char checksum;
} Packet;

// char calculateChecksum() { return '0'; }

void *sendPackets(void *args) { return 0; }

void *sendPacketsType1(void *args) {
  // send type1packets every 100ms
  return 0;
}
void *sendPacketsType2(void *args) {
  // send type1packets every 150ms
  return 0;
}

Packet createPacket() {
  Packet newPacket;
  return newPacket;
}

// void createConnection(){};

int main() {

  pthread_t type1Thread, type2Thread;

  pthread_create(&type1Thread, NULL, sendPacketsType1, NULL);
  pthread_create(&type2Thread, NULL, sendPacketsType2, NULL);
  pthread_join(type1Thread, NULL);
  pthread_join(type2Thread, NULL);

  return 0;
}
