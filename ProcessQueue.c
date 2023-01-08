#include "datu_egiturak.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/**
 * Ilara hutsik dagoen bueltatzen du,
 * Hutsik dago baldin eta 0 prozesua bakarrik dagoen
*/
u_int8_t isEmpty(struct ProcessQueue *PQ){
    return PQ->Next == PQ;
}

/**
 * Ilarako prozesu motzena aukeratzen du, 0 prozesua kontuan hartu gabe
*/
struct ProcessQueue *shortestProcess(struct ProcessQueue *PQ){
    struct ProcessQueue *head = PQ;
    struct ProcessQueue *currentPQ = PQ->Next;
    struct ProcessQueue *returnPQ = currentPQ->Next;
    while(currentPQ != head){

        if(currentPQ->data->PRIORITY  >  returnPQ->data->PRIORITY){
            returnPQ = currentPQ;
        }
        if(currentPQ->Next == head){
            break;
        }else{
            currentPQ = currentPQ->Next;
        }
    }
    return returnPQ;

}

/*
*Pasatzen zaion nodoa ilaratik ateratzen da
*/
void deletePQ(struct ProcessQueue *PQ){
   
    PQ->Previous->Next = PQ->Next;
}

/**
 * Ilara imprimatzen du
*/
void printList(struct ProcessQueue *PQ){
    struct ProcessQueue *head = PQ;
    struct ProcessQueue *currentPQ = PQ;
    printf("ProccesQueue :\n");
    while(currentPQ != NULL){
        printf("-----\n");
        printf("PID - %d - Priority - %d \n", currentPQ->data->ID, currentPQ->data->PRIORITY);
        
        
        if(currentPQ->Next == head){
            break;
        }else{
            currentPQ = currentPQ->Next;
        }
    }

}
/**
 * Ilara bukaeran prozesu berri bat sartzen du
*/
void insertLast(struct ProcessQueue *PQ, struct pcb *NewPcb){
    struct ProcessQueue *head = PQ;
    struct ProcessQueue *currentPQ = PQ;
    struct ProcessQueue *newNode = (struct ProcessQueue *)malloc(sizeof(struct ProcessQueue));
    newNode->Next = head; 
    newNode->data = NewPcb;
    while(currentPQ->Next != head){
        currentPQ = currentPQ->Next;
    }
    
    newNode->Previous = currentPQ;
    currentPQ->Next = newNode;
}

/**
 * Ilarako lehen elementua listatik atera eta bueltatzen du
 * 0 prozesua ez du kontuan hartzen
*/
struct pcb *deleteFirst(struct ProcessQueue *PQ){
    struct ProcessQueue *head = PQ;
    struct ProcessQueue *currentPQ = PQ->Next;
    struct pcb *returnData = PQ->Next->data;
    //currentPQ->data = currentPQ->Next->data;
    //currentPQ->Next = currentPQ->Next->Next;
    
    head->Next = currentPQ->Next;
    
    while(currentPQ->Next != head){
        currentPQ = currentPQ->Next;
    }
    currentPQ->Next = PQ;
    head->Previous = currentPQ;
    return returnData;
}

/**
 * 
struct pcb *deleteLast(struct ProcessQueue *PQ){
    struct ProcessQueue *currentPQ = PQ;
    struct ProcessQueue *head = PQ;
    if(currentPQ->data->ID == 0){
        return  NULL;
    }
    if(currentPQ->Next == head){
        return currentPQ->data;
    }
    while(currentPQ->Next->Next != head){
        currentPQ = currentPQ->Next;
    }
    struct pcb *returnData = currentPQ->Next->data;
    currentPQ->Next = head;
    return returnData;
}
*/
