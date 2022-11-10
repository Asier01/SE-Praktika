#ifndef datu_egiturak
#define datu_egiturak

struct pcb{
    int ID;
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


#endif