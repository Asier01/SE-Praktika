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

//
//ENTREGA -> Urtarrillak 13 - 23:55
//

//TODO:
//MAKEFILE egin kompilatzeko
//Modularizatu .c ezberdinetan


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
#define PRIORITY_SCHED 3

//Round robinerako quantum denbora
#define ROUND_ROBIN_QUANTUM 2


//Memoria fisikoa
#define PHYSICAL_MEMORY_SIZE 16777216 //2^24, 24 biteko heldibeak 
int PHYSICAL_MEMORY[PHYSICAL_MEMORY_SIZE];


//Memoria espazioak (placeholder balioak)
//#define KERNEL_SPACE_SIZE 1024 
#define KERNEL_ADDRESS 0xFF0000

#define DATA_SPACE_SIZE 4096
#define DATA_SPACE_ADDRESS 0x00100

#define TEXT_SPACE_SIZE 4096
#define TEXT_SPACE_ADDRESS 0x00200





int numCore;
int done = 0;
int tenp_kop = 0;
int currentPID = 1; //Hasierako prozesuaren ID-a
int kont_sched = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

sem_t sem_shceduler;
sem_t sem_proccesGenerator;
sem_t sem_execute;
sem_t sem_loader;


//Scheduling algoritmoaren defektuzko balioa
int scheduler_algorithm = FIFO;


//Prozesu lista hasieratu
struct ProcessQueue *PQ = NULL; 
struct cpuCore *coreList[1];


//struct cpuCore *core1= NULL;

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

//String baten substring bat lortzeko, p indizetik l indizera
void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
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
void *CPU(){
    pthread_mutex_lock(&mutex);
    int kont = 0;//Kontagailua hasieratu
    while(1){
        done++;

       //TODO: CPU-aren exekuzioak eginsdlksdohisd

        pthread_cond_signal(&cond1);
        pthread_cond_wait(&cond2, &mutex);
        kont++;
       
    }
}
*/


void execute_proccess(struct pcb *Process){
    int kodeLuzera = (Process->MEMORY_MANAGER.data - Process->MEMORY_MANAGER.code)/4;


    printf("------- KODE LUZERA ---%d \n", kodeLuzera);

    int kodeHelbFisiko = PHYSICAL_MEMORY[Process->MEMORY_MANAGER.pgb]; 
    int dataHelbFisiko = PHYSICAL_MEMORY[Process->MEMORY_MANAGER.pgb+1];
    printf("DATA HELBIDE FISIKOA --- %d\n", dataHelbFisiko);
    
    int erregList[16];
    
    /**
     * pgb-aren forma
     * adibidez:
     *  0xFFF0004 -> Helb-Fisiko-Text
     *  0xFFF0008 -> Helb-Fisiko-Data
    */
    char* currentCode = (char*)malloc(8 * sizeof(char));
    char* erregHelb = (char*)malloc(6 * sizeof(char));

    int erregHelbFis;
    char erreg;
    char erreg2;//add agindurako
    char erreg3;//add agindurako

    for(int i=0;i<kodeLuzera; i++){

        sprintf(currentCode, "%.8x", PHYSICAL_MEMORY[kodeHelbFisiko+i]);
        //printf("++++++++++++++++++%s\n", currentCode);
        //printf("\n---------------------------");
        switch (currentCode[0])
        {
            // -48 erabiltzen da zenbakizko balioa lortzeko, funtziona beharko zuen.

        case '0': // ld

            sprintf(erregHelb, "%s\n", currentCode + strlen(currentCode) - 6);//Helbide logikoa substring moduan lortu
            

            //printf("ERREG HELBIDE LOGIKOA %s\n", erregHelb);
            //printf("ERREG HELBIDE LOGIKOA %d\n", (int)strtol(erregHelb, NULL,16));

            erregHelbFis = dataHelbFisiko + ((int)strtol(erregHelb, NULL,16) - Process->MEMORY_MANAGER.data)/4;
            //printf("ERREG HELBIDE FISIKOA %d\n", erregHelbFis);
            erreg = currentCode[1]; //Zein erregistro erabiliko den aukeratu

            erregList[erreg - 48] = (__int32_t)PHYSICAL_MEMORY[erregHelbFis]; //Erregistroan kargatu
            printf("KARGATUTA --- %d \n",(__int32_t)PHYSICAL_MEMORY[erregHelbFis]);
            break;
        case '1': // st


            sprintf(erregHelb, "%s\n", currentCode + strlen(currentCode) - 6);//Helbidea substring moduan lortu

            erreg = currentCode[1]; //Zein erregistro erabiliko den aukeratu

            erregHelbFis = dataHelbFisiko + ((int)strtol(erregHelb, NULL,16) - Process->MEMORY_MANAGER.data)/4;

            PHYSICAL_MEMORY[erregHelbFis] = erregList[erreg -48];
            printf("KALKULUA EMAITZA -----> %d\n", erregList[erreg -48]);
            break;
        case '2': // add
            erreg = currentCode[1]; //Beharrezko erregistroak hartu
            erreg2 = currentCode[2];
            erreg3 = currentCode[3];
                
            erregList[erreg-48] = erregList[erreg2-48] + erregList[erreg3-48];  //Kalkulua egin
            break;

        default:
            //printf("EMAITZA -------%d\n--------", PHYSICAL_MEMORY[(int)strtol(erregHelb, NULL,16)]);   
            break;
        }
        
    }
    printf("EXEKUZIOA BUKATUTA\n");
   
    //sem_post(&sem_execute); 
    
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


    int currentKernelAddress = KERNEL_ADDRESS;
    

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

    int helb_fis = 0;
    
    while(fp!=NULL){ 
         progKont++;
         while(1){
             struct pcb *newProcces = (struct pcb*)malloc(sizeof(struct pcb));   //Prozesu berri bat (pcb bat) sortu

             //sem_wait(&sem_proccesGenerator);    //Tenporizadoreari itxaron

             //Aginduen helbide birtuala lortu
             read = getline(&line, &len, fp);
             token = strtok(line, " "); 
             token = strtok(NULL, " "); //Bigarren tokena lortu


             textAddr = (int)strtol(token, NULL, 16);//String-a balio hexadezimaleko int-era bihurtu(4 byte okupatzeko eta ez 8)
            // printf("%.8X\n", textAddr);
            // printf("-----------------------\n");


             //Datuen helbide birtuala lortu
             read = getline(&line, &len, fp);

             token = strtok(line, " ");

             token = strtok(NULL, " ");//Bigarren tokena lortu


             dataAddr = (int)strtol(token, NULL,16);
            // printf("%.8x\n", dataAddr);
            // printf("-----------------------\n");

             //8 karaktereko string batera printeatu (for the future) 
             /**---------------------------------------------------------------------------------
             char* hexfinal = (char*)malloc(8 * sizeof(char));;
             sprintf(hexfinal, "%.8x");
             printf("%s\n", hexfinal);
             **/
             //-------------------------------------------------------------------------------

             helbLog = 0x0;
             
             PHYSICAL_MEMORY[currentKernelAddress] = helb_fis;
             
             //printf("KodeSegmentua\n");
             while((getline(&line, &len, fp)) != -1) {

                helbLog = helbLog+4;
                PHYSICAL_MEMORY[helb_fis] = (int)strtol(line, NULL,16);
                helb_fis++;
                if(helbLog == dataAddr){
                    break;
                }


             }

             PHYSICAL_MEMORY[currentKernelAddress+1] = helb_fis;

             //printf("DatuSegmentua\n");
             while ((getline(&line, &len, fp)) != -1) {
                PHYSICAL_MEMORY[helb_fis] = (int)strtol(line, NULL,16);
                printf("KARGATUTAKO DATUAK - %d \n",PHYSICAL_MEMORY[helb_fis]);
                helb_fis++;
                
             }

             //PHYSICAL_MEMORY[helb_fis] = 0;
             //PHYSICAL_MEMORY[helb_fis+1] = 0 + dataAddr/4; 

             newProcces->MEMORY_MANAGER.pgb =  currentKernelAddress;
            printf("+++++++++++++++++++++ %d\n", currentKernelAddress);
            printf("+++++++++++++++++++++ %d\n", PHYSICAL_MEMORY[newProcces->MEMORY_MANAGER.pgb]);

             newProcces->ID = currentPID++;  //Bere atributuak esleitu
             newProcces->STATE = WAIT;
             newProcces->MEMORY_MANAGER.code = textAddr;
             newProcces->MEMORY_MANAGER.data = dataAddr;

             newProcces->PRIORITY = random() % 20+1;
             
             //(__int32_t) datuen balioak lortzeko
             //newProcces->EXEC_TIME =1 + rand() % 5;

            currentKernelAddress = currentKernelAddress+2;



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
    sem_post(&sem_loader);
}       
/**     
 * Tenporizadoreak deitzen dion funtzioa
*/
void *scheduler(){
    while(1){

        printf("schedulerrari deitu zaio \n");

        struct cpuCore *currentCore;
        for(int i=0; i<numCore; i++){
            if(coreList[i]->executableProcess==NULL){
                currentCore = coreList[i];
                printf("%d CORE-A AUKERATUTA\n", i);
                break;
            }
        }

        if(currentCore==NULL){
            sem_wait(&sem_shceduler);
        }


           //Tenporizadoreari itxaron
         

        struct pcb *currentProcces;

        if(!isEmpty(PQ)){
            switch(scheduler_algorithm){//Scheduling algoritmoaren arabera...

                case FIFO:
                    currentProcces = deleteFirst(PQ); //Listako lehen elementua hartu eta listatik kendu(0 prozesua ez da hartzen)
                    currentProcces->STATE = RUN;
                    printf("EXEKUTATZEN %d PROZESUA --- %d CORE-AREKIN \n", currentProcces->ID, currentCore->ID);
                    
                    currentCore->executableProcess = currentProcces;

                    break;

                /**
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
                **/
                case PRIORITY_SCHED:
                        
                        struct ProcessQueue *shortestPQ = shortestProcess(PQ); //Prozesu motzena aukeratu
                        
                        deletePQ(shortestPQ);

                        currentProcces = shortestPQ->data;  
                        printf("EXEKUTATZEN %d PROZESUA \n", currentProcces->ID);
                        //printf("%d \n", currentProcces->ID);
                        //currentProcces = shortestProcess(PQ);
                        
                         //Prozesua ilaratik atera

                        currentCore->executableProcess = currentProcces;

                        
                        
                        
                        break;

            }
            //printList(PQ); //Prozesu ilara printeatu
        }else{
            printf("EZ DAGO EXEKUTATZEKO PROZESURIK\n");
            sleep(1);
        }
        kont_sched = 0;
        
       
        
    }
}


void *cpuExecute(void *coreID){
    
    
    cpuid = (int )coreID;
    struct cpuCore *core = (struct cpuCore*)malloc(sizeof(struct cpuCore));
    core->ID = cpuid;
    core->EXECUTING = 0;
    core->IR=0;
    core->PC=0;
    core->PTRB;
   

    coreList[cpuid] = core;

   

    printf("%d CORE-A ONDO SORTUTA \n", cpuid);
    cpuid++;
    int kont = 0;//Kontagailua hasieratu
    pthread_mutex_lock(&mutex);
    while(1){
        
        int zikloKop = 0;
        while(core->executableProcess!=NULL){
            done++;

            execute_proccess(core->executableProcess);

            core->executableProcess = NULL;

            pthread_cond_signal(&cond1);
            pthread_cond_wait(&cond2, &mutex);
            zikloKop++;
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
        if(strcmp(argv[1],"PRIORITY")==0){
            printf("SCHEDULING ALGORITMOA -> PRIORITY \n");
            scheduler_algorithm = PRIORITY_SCHED;
        }
    }else{
        printf("DEFEKTUZKO SCHEDULING ALGORITMOA -> FIFO \n");
    }
    
    numCore =atoi(argv[2]); 
    coreList[numCore];

    printf("-----HASIERATUTA %d CORE--------\n", numCore);

    /*TODO:
        Implementatu core ezberdinen hasieraketa
        Hari bat sortu Core bakoitzeko 
        Mutex bidez sinkronizatu (maybe)
    */

    for(int i=0; i<PHYSICAL_MEMORY_SIZE; i++){
        PHYSICAL_MEMORY[i] = malloc(4);
        PHYSICAL_MEMORY[i] = INT_MAX; // Libre dauden helbideak INT_MAX izango dute
    }
    
    //PHYSICAL_MEMORY[0] = 234346346346363095390864457645;
    //printf("%d \n",PHYSICAL_MEMORY[0]);
    
    //Core-ak hasieratu
    
    

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
    
    pthread_create(&procGen, NULL, loader, NULL );

    sem_wait(&sem_loader);

    tenp_kop = 0;
    pthread_create(&erloj, NULL, erlojua, NULL);

    pthread_create(&tenp2, NULL, tenporizadorea_sched, NULL);
    tenp_kop++;

    void *coreID = 0;

     for(int i=0; i<numCore; i++){
        pthread_t core;
        
        printf("hasieratuta %d core-a\n", i);
        pthread_create(&core, NULL, cpuExecute, coreID);
        coreID = coreID+1;
        tenp_kop++;

    }

    
    //pthread_create(&tenp1, NULL, tenporizadorea_proccessGen, NULL);
    //tenp_kop++;

    pthread_create(&sched, NULL, scheduler, NULL);

    

   
    pthread_join(erloj, NULL);

}
