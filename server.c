#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.

#define PORT 6000
#define TYPE_1 1
#define TYPE_2 2

typedef struct Packet {
  int type;
  int sequenceNumber;
  char payload[1024];
  char checksum;
} Packet;

struct sockaddr_in reciver_addr;
int sock_decr;

char calculateChecksum(const char *s) {
  char checksum = 0;
  for (int i = 0; i < 1024; i++) {
    checksum ^= s[i];
  }
  return checksum;
}

Packet createPacket(int seqNum, int type) {
  Packet newPacket;
  newPacket.type = type;
  strcpy(newPacket.payload, "I'm a Packet!!\n");
  newPacket.sequenceNumber = seqNum;
  newPacket.checksum = calculateChecksum(newPacket.payload);
  return newPacket;
}

void *sendPacketsType1(void *args) {
  // send type1packets every 100ms
  int seqNum = 1;
  while (1) {
    usleep(100000);
    struct Packet currPacket = createPacket(seqNum, TYPE_1);
    printf("sending packet type %d\n, sequenceNumber %d", currPacket.type,
           currPacket.sequenceNumber);
    sendto(sock_decr, &currPacket, sizeof(struct Packet), 0,
           (struct sockaddr *)&reciver_addr, sizeof(struct sockaddr_in));
    seqNum++;
  }
  return 0;
}
void *sendPacketsType2(void *args) {
  // send type1packets every 150ms
  int seqNum = 1;
  while (1) {
    usleep(150000);
    struct Packet currPacket = createPacket(seqNum, TYPE_2);
    printf("sending packet type %d\n, sequenceNumber %d", currPacket.type,
           currPacket.sequenceNumber);
    sendto(sock_decr, &currPacket, sizeof(struct Packet), 0,
           (struct sockaddr *)&reciver_addr, sizeof(struct sockaddr_in));
    seqNum++;
  }
  return 0;
}

void createConnection() {
  sock_decr = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_decr == -1) {
    printf("socket creation Failed!!\n");
    exit(0);
  }
  reciver_addr.sin_family = AF_INET;
  reciver_addr.sin_port = htons(PORT);
  reciver_addr.sin_addr.s_addr = INADDR_ANY;
};

int main() {

  pthread_t type1Thread, type2Thread;

  createConnection();

  pthread_create(&type1Thread, NULL, sendPacketsType1, NULL);
  pthread_create(&type2Thread, NULL, sendPacketsType2, NULL);
  pthread_join(type1Thread, NULL);
  pthread_join(type2Thread, NULL);

  return 0;
}
