#ifndef datu_egiturak
#define datu_egiturak


struct mm{
    int pgb; //orri-taularen helbide fisikoa
    int code; //kodearen segmentuaren helbide birtuala
    int data;   //datuen segmentuaren helbide birtuala
};
struct pcb{
    int ID;
    struct mm MEMORY_MANAGER;
    int STATE;
    int EXEC_TIME;
    // int PRIORITY;
    // int PC;
    // ....
};

struct ProcessQueue{
        struct ProcessQueue *Next;
        struct ProcessQueue *Previous;
        struct pcb *data;
};



struct cpuCore{ 
    int ID;

};

#endif