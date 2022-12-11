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


//Memoria fisikoa
#define PHYSICAL_MEMORY_SIZE 16777216 //2^24, 24 biteko heldibeak 
int PHYSICAL_MEMORY[PHYSICAL_MEMORY_SIZE];


//Memoria espazioak (placeholder balioak)
#define KERNEL_SPACE_SIZE 1024 
#define KERNEL_ADDRESS 0xF00000

#define DATA_SPACE_SIZE 4096
#define DATA_SPACE_ADDRESS 0x00100

#define TEXT_SPACE_SIZE 4096
#define TEXT_SPACE_ADDRESS 0x00200

struct cpuCore* coreList[];

int erregList[16];


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


struct cpuCore *core1= NULL;

//CPU-ak exekutatuko duen prozesua
struct pcb *ExecProcess = NULL;
int cpuid = 0;

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
        
    //TODO:
    //Erlojua konpondu, ez dio schedulerrari deitzen.
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
/**
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
*/
void execute_proccess(struct pcb *Process){
    int kodeLuzera = (Process->MEMORY_MANAGER.data - Process->MEMORY_MANAGER.code)/4;

    int kodeHelbFisiko = PHYSICAL_MEMORY[Process->MEMORY_MANAGER.pgb]; 
    int dataHelbFisiko = PHYSICAL_MEMORY[Process->MEMORY_MANAGER.pgb+1];
    /**
     * pgb-aren forma
     * adibidez:
     *  0xFFF0004 -> Helb-Fisiko-Text
     *  0xFFF0008 -> Helb-Fisiko-Data
    */
    char* currentCode = (char*)malloc(8 * sizeof(char));;
    

    for(int i=0; i++; i<kodeLuzera){
        sprintf(currentCode, "%.8x", PHYSICAL_MEMORY[kodeHelbFisiko+i]);
        
        //TODO:
        //Kodearen balio hexadezimala string moduan izanda
        //subString-ak erabili agindua zein den jakiteko eta exekutatzeko
        //Erregistro listataz baliatuz 'erregList[]'

        //switch(currentCode)
        
    }

}

//Tenporizadoreak deitzen duen funtzioa, prozesu berri bat sortzen du.
void *loader(){
    
    
    FILE *fp;
    char* filename =(char *) malloc(11*sizeof(char)); 
    filename = "prog000.elf"; //Lehenengo fitxategia -> Fitxategiak ordenean nomenklatura berdina izan behar dute ondo irakurtzeko
                                                    //  prog000.elf ; prog001.elf ; prog002.elf ...... 
    fp = fopen(filename, "r" );
    
    int dataAddr;
    int textAddr;
    

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    char* token = NULL;

    int helbLog = 0x0;

    if(fp == NULL){
        printf("Ezin izan da ondo irakurri fitxategia !!! \n");
        exit(EXIT_FAILURE);
    }
    int progKont = 0;
    char * progNumStr = (char *)malloc(11*sizeof(char));
    //
    //Agindu transkripzioa egin
    //
    while(fp!=NULL){ 
         progKont++;
         while(1){//while honek ez du ezer egiten oraingoz

             //sem_wait(&sem_proccesGenerator);    //Tenporizadoreari itxaron

             //Aginduen helbide birtuala lortu
             read = getline(&line, &len, fp);
             token = strtok(line, " "); 
             token = strtok(NULL, " "); //Bigarren tokena lortu


             textAddr = (int)strtol(token, NULL, 16);//String-a balio hexadezimaleko int-era bihurtu(4 byte okupatzeko eta ez 8)
             printf("%.8X\n", textAddr);
             printf("-----------------------\n");


             //Datuen helbide birtuala lortu
             read = getline(&line, &len, fp);

             token = strtok(line, " ");

             token = strtok(NULL, " ");//Bigarren tokena lortu


             dataAddr = (int)strtol(token, NULL,16);
             printf("%.8x\n", dataAddr);
             printf("-----------------------\n");

             //8 karaktereko string batera printeatu (for the future) 
             /**---------------------------------------------------------------------------------
             char* hexfinal = (char*)malloc(8 * sizeof(char));;
             sprintf(hexfinal, "%.8x");
             printf("%s\n", hexfinal);
             **/
             //-------------------------------------------------------------------------------

             helbLog = 0x0;


             //printf("KodeSegmentua\n");
             while((getline(&line, &len, fp)) != -1) {

                //printf("%s \n", line);

                helbLog = helbLog+4;

                 if(helbLog == dataAddr){
                     break;
                 }


             }
             //printf("DatuSegmentua\n");
             while ((getline(&line, &len, fp)) != -1) {
               //  printf("%s \n", line);
             }



             struct pcb *newProcces = (struct pcb*)malloc(sizeof(struct pcb));   //Prozesu berri bat (pcb bat) sortu
             newProcces->ID = currentPID++;  //Bere atributuak esleitu
             newProcces->STATE = WAIT;
             newProcces->MEMORY_MANAGER.code = textAddr;
             newProcces->MEMORY_MANAGER.data = dataAddr;
             //(__int32_t) datuen balioak lortzeko
             //newProcces->EXEC_TIME =1 + rand() % 5;




             insertLast(PQ, newProcces);
             printf("prozesu berri bat sortu da, PID -> %d \n", newProcces->ID);
            

             fclose(fp);
             sprintf(progNumStr, "prog%.3d.elf", progKont); //Hurrengo fitxategiaren izena 
             //printf("%s\n", progNumStr);
             filename = progNumStr; //Filename bakarrik erabiltzen bada segFault ematen du, ni diea zergaitik.
             fp = fopen(filename, "r"); //Hurrengo fitxategia zabaldu, ez bada existitzen ez du hurrengo iterazioa burutuko
             break;
         }
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
                    
                    /**
                     *sleep(currentProcces->EXEC_TIME); //Prozesua "exekutatu", oraingoz sleep bat 
                     */
                    
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


void *cpuExecute(){
    //Ponlo en el main maybeeee
    //cpuid+=1;
    //struct cpuCore *core = (struct cpuCore*)malloc(sizeof(struct cpuCore));
    //core->ID = cpuid;
    //core->EXECUTING = 0;
    //core->IR=0;
    //core->PC=0;
    //core->PTRB;

    int kont = 0;//Kontagailua hasieratu
    pthread_mutex_lock(&mutex);
    while(1){
        
        kont = 0;
        while(core1->EXECUTING){
            done++;
            
            //........
            if(kont>=ROUND_ROBIN_QUANTUM && scheduler_algorithm ==ROUND_ROBIN){
                core1->EXECUTING = 0;
            }

            pthread_cond_signal(&cond1);
            pthread_cond_wait(&cond2, &mutex);
            kont++;
        }


        if(kont>=ROUND_ROBIN_QUANTUM && scheduler_algorithm ==ROUND_ROBIN){
            pthread_cond_signal(&cond1);
            pthread_cond_wait(&cond2, &mutex);
        }
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
    
    /*TODO:
        Implementatu core ezberdinen hasieraketa
        Hari bat sortu Core bakoitzeko 
        Mutex bidez sinkronizatu (maybe)
    */

    for(int i=0; i<PHYSICAL_MEMORY_SIZE; i++){
        PHYSICAL_MEMORY[i] = malloc(4);
        PHYSICAL_MEMORY[i] = INT_MAX;
    }
    
    //PHYSICAL_MEMORY[0] = 234346346346363095390864457645;
    //printf("%d \n",PHYSICAL_MEMORY[0]);
    
    //Core-ak hasieratu
    
    core1 = (struct cpuCore *)malloc(sizeof(struct cpuCore));

    //Prozesu ilara hasieratu, ilara zirkular bat moduan hartu da
    PQ = (struct ProcessQueue *)malloc(sizeof(struct ProcessQueue));
    PQ->Next = PQ;
    PQ->data = (struct pcb*)malloc(sizeof(struct pcb));
    PQ->data->ID = 0;
    PQ->data->STATE = 0;
    PQ->Previous = PQ;

/**
    for(int i=0; i<4; i++){
        coreList[i] = (struct cpuCore*)malloc(sizeof(struct cpuCore));
    }
   */     




    ExecProcess = PQ->data;

    //Hariak hasieratu eta deitu

    pthread_t erloj, tenp1, tenp2, sched, procGen, coreExec1;
    tenp_kop = 0;
    pthread_create(&erloj, NULL, erlojua, NULL);

    pthread_create(&tenp2, NULL, tenporizadorea_sched, NULL);
    tenp_kop++;
    
    //pthread_create(&tenp1, NULL, tenporizadorea_proccessGen, NULL);
    //tenp_kop++;

    pthread_create(&sched, NULL, scheduler, NULL);

    pthread_create(&procGen, NULL, loader, NULL );


    pthread_create(&coreExec1, NULL, cpuExecute, NULL);


    pthread_join(erloj, NULL);

}
