#include "datu_egiturak.h"
#include "ProcessQueue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <limits.h>
#include <math.h>
#include <string.h>

//Schedulerrari eta prozesu generadoreari deitzeko frekuentzia
#define SCHEDULER_FRECUENCY 100000
#define PROCCESS_GENERATOR_FRECUENCY 150000


//Prozesuen state posibleak
#define READY 1
#define RUN   2
#define WAIT  3

//Scheduling algoritmoak
#define FIFO 1
#define ROUND_ROBIN 2
#define SJF 3

//Round robinerako quantum denbora
#define ROUND_ROBIN_QUANTUM 2

int done = 0;
int tenp_kop = 0;
int currentPID = 1; //Hasierako prozesuaren ID-a
int kont_sched = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

sem_t sem_shceduler;
sem_t sem_proccesGenerator;


//Scheduling algoritmoaren defektuzko balioa
int scheduler_algorithm = FIFO;


//Prozesu lista hasieratu
struct ProcessQueue *PQ = NULL;

//Bi balioen minimoa lortzeko funtzioa. Ez galdetu zergaitik inplementatu nuen, ez naiz gogoratzen, baina funtzionatzen du
int min(int x, int y){
    if(x<y){
        return x;
    }else{
        return y;
    }
}


//Erlojua
void *erlojua(){
    
    while(1){
        pthread_mutex_lock(&mutex);
        while(done<tenp_kop){
            pthread_cond_wait(&cond1, &mutex);
            //....
        }
        
        done = 0;

        pthread_cond_broadcast(&cond2);
        pthread_mutex_unlock(&mutex);
        

    }
}

//Schedulerrari deitzeko tenporizadorea
void *tenporizadorea_sched(){
    pthread_mutex_lock(&mutex);
    //Kontagailua hasieratu
    while(1){
        done++;

        if(kont_sched>=SCHEDULER_FRECUENCY){  //Kontagailua frekuentziaren konstantera heltzean, schedulerrari deitu
            sem_post(&sem_shceduler);  //Schedulerrari deitu
            kont_sched=0; //Kontagailua berriro zero bihurtu
        }

        pthread_cond_signal(&cond1);
        pthread_cond_wait(&cond2, &mutex);
        kont_sched++;
       
    }
}

//Prozesu sortzaileari deitzeko tenporizadorea
void *tenporizadorea_proccessGen(){
    pthread_mutex_lock(&mutex);
    int kont = 0;//Kontagailua hasieratu
    while(1){
        done++;

        if(kont>=PROCCESS_GENERATOR_FRECUENCY){//Kontagailua frekuentziaren konstantera heltzean, prozesu sortzaileari deitu
            sem_post(&sem_proccesGenerator);  //Prozesu sortzaileari deitu
            kont=0;//Kontagailua berriro zero bihurtu
        }

        pthread_cond_signal(&cond1);
        pthread_cond_wait(&cond2, &mutex);
        kont++;
       
    }
}

//Tenporizadoreak deitzen duen funtzioa, prozesu berri bat sortzen du.
void *processGenerator(){
    while(1){
        sem_wait(&sem_proccesGenerator);    //Tenporizadoreari itxaron
        struct pcb *newProcces = (struct pcb*)malloc(sizeof(struct pcb));   //Prozesu berri bat (pcb bat) sortu
        newProcces->ID = currentPID++;  //Bere atributuak esleitu
        newProcces->STATE = WAIT;
        newProcces->EXEC_TIME =1 + rand() % 5;

        insertLast(PQ, newProcces);
        printf("prozesu berri bat sortu da, PID -> %d \n", newProcces->ID);
        
        
    }
   
}
/**
 * Tenporizadoreak deitzen dion funtzioa
*/
void *scheduler(){
    while(1){
        sem_wait(&sem_shceduler);   //Tenporizadoreari itxaron
        printf("schedulerrari deitu zaio \n");  

        struct pcb *currentProcces;
        if(!isEmpty(PQ)){
            switch(scheduler_algorithm){//Scheduling algoritmoaren arabera...

                case FIFO:
                    currentProcces = deleteFirst(PQ); //Listako lehen elementua hartu eta listatik kendu(0 prozesua ez da hartzen)
                    currentProcces->STATE = RUN;
                    printf("EXEKUTATZEN %d PROZESUA \n", currentProcces->ID);
                    sleep(currentProcces->EXEC_TIME); //Prozesua "exekutatu", oraingoz sleep bat
                    break;

                case ROUND_ROBIN:
                    currentProcces = deleteFirst(PQ); //Listako lehen elementua hartu eta listatik kendu(0 prozesua ez da hartzen)
                    printf("EXEKUTATZEN %d PROZESUA \n", currentProcces->ID);
                    if(currentProcces->EXEC_TIME<=ROUND_ROBIN_QUANTUM){ //Quantuma baino denbora gutxiago behar badu...
                        
                        currentProcces->STATE = RUN;
                        sleep(min(currentProcces->EXEC_TIME, ROUND_ROBIN_QUANTUM));//Guztiz exekutatu
                        //free(currentProcces);
                        sleep(ROUND_ROBIN_QUANTUM - currentProcces->EXEC_TIME);//Quantumari gelditzen zaioa itxaron

                    }else{ //Bestela...
                        currentProcces->STATE = RUN;
                        sleep(ROUND_ROBIN_QUANTUM);//Quantum denboran 'exekutatu'
                        currentProcces->EXEC_TIME = currentProcces->EXEC_TIME-ROUND_ROBIN_QUANTUM; //Behar duen exekuzioa denbora eguneratu
                        insertLast(PQ, currentProcces);//Listaren bukaeran gehitu
                        currentProcces->STATE=WAIT;
                    }
                    break;
                case SJF:
                        
                        struct ProcessQueue *shortestPQ = shortestProcess(PQ); //Prozesu motzena aukeratu
                        
                        currentProcces = shortestPQ->data;  
                        printf("EXEKUTATZEN %d PROZESUA \n", currentProcces->ID);
                        //printf("%d \n", currentProcces->ID);
                        //currentProcces = shortestProcess(PQ);
                        currentProcces->STATE = RUN;
                        
                        sleep(currentProcces->EXEC_TIME); //Prozesu hori 'exekutatu'
                        
                        deletePQ(shortestPQ); //Prozesua ilaratik atera

            }
        }
        kont_sched = 0;
        
        printList(PQ); //Prozesu ilara printeatu
        
    }
}




int main(int argc, char *argv[]){
    //Parametro moduan hartu scheduling algoritmoa, defektuz FIFO da
    if(argc>1){
        //printf("%s \n", argv[1]);
        if(strcmp(argv[1],"FIFO")==0){
            printf("SCHEDULING ALGORITMOA -> FIFO \n");
            scheduler_algorithm = FIFO;
        }
        if(strcmp(argv[1],"ROUNDROBIN")==0){
            printf("SCHEDULING ALGORITMOA -> ROUNDROBIN \n");
            scheduler_algorithm = ROUND_ROBIN;
        }
        if(strcmp(argv[1],"SJF")==0){
            printf("SCHEDULING ALGORITMOA -> SJF \n");
            scheduler_algorithm = SJF;
        }
    }else{
        printf("DEFEKTUZKO SCHEDULING ALGORITMOA -> FIFO \n");
    }
    

    //Prozesu ilara hasieratu, ilara zirkular bat moduan hartu da
    PQ = (struct ProcessQueue *)malloc(sizeof(struct ProcessQueue));
    PQ->Next = PQ;
    PQ->data = (struct pcb*)malloc(sizeof(struct pcb));
    PQ->data->ID = 0;
    PQ->data->STATE = 0;
    PQ->Previous = PQ;

    //Hariak hasieratu eta deitu

    pthread_t erloj, tenp1, tenp2, sched, procGen;
    tenp_kop = 0;
    pthread_create(&erloj, NULL, erlojua, NULL);

    pthread_create(&tenp2, NULL, tenporizadorea_sched, NULL);
    tenp_kop++;
    
    pthread_create(&tenp1, NULL, tenporizadorea_proccessGen, NULL);
    tenp_kop++;

    pthread_create(&sched, NULL, scheduler, NULL);

    pthread_create(&procGen, NULL, processGenerator, NULL );

    
    
    pthread_join(erloj, NULL);

}
