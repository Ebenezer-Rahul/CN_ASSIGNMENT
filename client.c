#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details

#define PORT 6000
#define TYPE_1 1
#define TYPE_2 2

typedef struct Packet {
  int type;
  int sequenceNumber;
  char payload[1024];
  char checksum;
} Packet;

typedef struct LL {
  Packet *packet;
  struct LL *next;
} LL;

typedef struct Queue {
  LL *front;
  LL *back;
} Queue;

LL temp;

Queue *createQueue() {
  Queue *newQueue = (Queue *)malloc(sizeof(Queue));
  newQueue->back = &temp;
  newQueue->front = &temp;
  temp.next = NULL;
  return newQueue;
}

void insertIntoQueue(Queue *q, Packet curr) {
  LL *currNode = (LL *)malloc(sizeof(LL));
  Packet *cpyPacket = (Packet *)malloc(sizeof(Packet));
  *cpyPacket = curr;
  currNode->packet = cpyPacket;
  currNode->next = NULL;
  q->back->next = currNode;
  q->back = currNode;
  return;
}

Packet *popFromQueue(Queue *q) {
  if (q->front->next == NULL) {
    printf("Queue is empty\n");
    exit(0);
  }
  Packet *curr = q->front->next->packet;
  q->front->next = q->front->next->next;
  if (q->front->next == NULL) {
    q->back = &temp;
  }
  return curr;
}

struct sockaddr_in sender_addr;
int sock_decr;

void createConnection() {
  sock_decr = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_decr == -1) {
    printf("socket creation Failed!!\n");
    exit(0);
  }
  sender_addr.sin_family = AF_INET;
  sender_addr.sin_port = htons(PORT);
  sender_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock_decr, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) ==
      -1) {
    printf("socket binding Failed!!\n");
    exit(0);
  };
};

char calculateChecksum(const char *s) {
  char checksum = 0;
  for (int i = 0; i < 1024; i++) {
    checksum ^= s[i];
  }
  return checksum;
}

// global Variables
Packet inProcessType1Packet;
Packet inProcessType2Packet;

Queue *type1packets;
Queue *type2packets;

sem_t signalType1ProcessingThread, signalType2ProcessingThread;
sem_t reportLock;
sem_t type1QueueLock, type2QueueLock;

struct Report {
  int type1Packets;
  int type2Packets;
} Report;

struct Report status_report;

int checkforErrors(struct Packet *currPacket) {
  if (currPacket->checksum != calculateChecksum(currPacket->payload)) {
    printf("Error in Packet of type %d, and sequenceNumber : %d\n ",
           currPacket->type, currPacket->sequenceNumber);
    return 0;
  }
  return 1;
}

unsigned int sender_addr_len = sizeof(sender_addr);

void *recievePackets(void *args) {
  printf("recievePackets\n");

  type1packets = createQueue();
  type2packets = createQueue();

  while (1) {
    Packet currPacket;

    recvfrom(sock_decr, &currPacket, sizeof(struct Packet), 0,
             (struct sockaddr *)&sender_addr, &sender_addr_len);

    if (!checkforErrors(&currPacket)) {
      continue;
    };

    if (currPacket.type == 1) {

      sem_wait(&type1QueueLock);
      insertIntoQueue(type1packets, currPacket);
      sem_post(&type1QueueLock);

      sem_wait(&reportLock);
      status_report.type1Packets++;
      sem_post(&reportLock);

      sem_post(&signalType1ProcessingThread);

    } else if (currPacket.type == 2) {
      sem_wait(&type2QueueLock);
      insertIntoQueue(type2packets, currPacket);
      sem_post(&type2QueueLock);

      sem_wait(&reportLock);
      status_report.type2Packets++;
      sem_post(&reportLock);

      sem_post(&signalType2ProcessingThread);
    } else {

      printf("ERROR UNKNOWN TYPE RECIEVED\n");
      exit(1);
    }
  }

  return 0;
}

void *processPacket1() {
  while (1) {
    sem_wait(&signalType1ProcessingThread);
    sem_wait(&type1QueueLock);
    Packet *curr = popFromQueue(type1packets);
    printf("type1 sequenceNumber:%d is RECIEVED\n", curr->sequenceNumber);
    sem_post(&type1QueueLock);
    // free(curr);
  };

  return 0;
}
void *processPacket2() {
  while (1) {
    sem_wait(&signalType2ProcessingThread);
    sem_wait(&type2QueueLock);
    Packet *curr = popFromQueue(type2packets);
    printf("type2 sequenceNumber:%d is RECIEVED\n", curr->sequenceNumber);
    sem_post(&type2QueueLock);
    // free(curr);
  }
  return 0;
}

void *reportStatuts() {
  while (1) {
    usleep(300000);
    sem_wait(&reportLock);
    printf("curr status is \ntype1packets count :%d\n, type2packets count:%d\n",
           status_report.type1Packets, status_report.type2Packets);
    sem_post(&reportLock);
  }
  return 0;
}

// void createConnection(){};

int main() {

  pthread_t recievingThread, type1ProcessingThread, type2ProcessingThread,
      statusReportingThread;

  struct Report currStatus;

  createConnection();
  memset(&currStatus, 0, sizeof(currStatus));

  sem_init(&reportLock, 0, 1);
  sem_init(&type1QueueLock, 0, 1);
  sem_init(&type2QueueLock, 0, 1);
  sem_init(&signalType1ProcessingThread, 0, 0);
  sem_init(&signalType2ProcessingThread, 0, 0);

  pthread_create(&recievingThread, NULL, recievePackets, NULL);
  pthread_create(&type1ProcessingThread, NULL, processPacket1, NULL);
  pthread_create(&type2ProcessingThread, NULL, processPacket2, NULL);
  pthread_create(&statusReportingThread, NULL, reportStatuts, NULL);

  pthread_join(recievingThread, NULL);
  pthread_join(type1ProcessingThread, NULL);
  pthread_join(type2ProcessingThread, NULL);
  pthread_join(statusReportingThread, NULL);

  return 0;
}
