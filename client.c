#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details

typedef struct Packet {
  int type;
  int sequenceNumber;
  char payload[1024];
  char checksum;
} Packet;

// char calculateChecksum() { return '0'; }

// global Variables
Packet inProcessType1Packet;
Packet inProcessType2Packet;

sem_t signalType1ProcessingThread, signalType2ProcessingThread;
sem_t reportLock;

struct Report {
  int type1Packets;
  int type2Packets;
} Report;

void *recievePackets(void *args) {

  while (1) {
    Packet currPacket;

    // recive
    //
    //
    // checkForErrors

    if (currPacket.type == 1) {
      inProcessType1Packet = currPacket;
      // signaltype1
      // increment type1packets report
    } else if (currPacket.type == 2) {
      inProcessType2Packet = currPacket;
      // signal type2
      // increment type2packets report
    } else {

      printf("ERROR UNKNOWN TYPE RECIEVED\n");
      exit(1);
    }
  }

  return 0;
}

int checkforErrors(Packet curr) { return 0; }

void processPacket(Packet curr) {}

void reportStatuts() {}

void *recievePacketsType2(void *args) {
  // send type1packets every 100ms
  return 0;
}

// void createConnection(){};

int main() {

  pthread_t recievingThread, type1ProcessingThread, type2ProcessingThread,
      statusReportingThread;

  struct Report currStatus;

  memset(&currStatus, 0, sizeof(currStatus));

  pthread_create(&recievingThread, NULL, recievePackets, NULL);
  pthread_create(&type1ProcessingThread, NULL, recievePacketsType2, NULL);
  pthread_create(&type2ProcessingThread, NULL, recievePacketsType2, NULL);
  pthread_create(&statusReportingThread, NULL, recievePacketsType2, NULL);

  pthread_join(recievingThread, NULL);
  pthread_join(type1ProcessingThread, NULL);
  pthread_join(type2ProcessingThread, NULL);
  pthread_join(statusReportingThread, NULL);

  return 0;
}
