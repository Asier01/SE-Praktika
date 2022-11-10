#ifndef ProcessQueue_H
#define ProcessQueue_H

#include "datu_egiturak.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

u_int8_t isEmpty(struct ProcessQueue *PQ);

struct ProcessQueue *shortestProcess(struct ProcessQueue *PQ);

void deletePQ(struct ProcessQueue *deletePQ);

void insertLast(struct ProcessQueue *PQ, struct pcb *NewPcb);

void instertFirst(struct ProcessQueue **PQ, struct pcb *NewPcb);

int deleteByPID(struct ProcessQueue *PQ, int PID);

struct pcb *deleteFirst(struct ProcessQueue *PQ);

//struct pcb *deleteLast(struct ProcessQueue *PQ);

// void initializeList(struct ProcessQueue *PQ);
void printList(struct ProcessQueue *PQ);


#endif