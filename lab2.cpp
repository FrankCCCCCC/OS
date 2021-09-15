#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <queue>
#include <list>
#include <stack>
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

//©CSCI-GA.2250-001 Spring-2021 Lab 2
//NetID: zc2244
//NAME: Zhiliang Chen

//Part of Creating enumeration 

typedef enum {
    STATE_CREATED,
    STATE_RUNNING,
    STATE_BLOCKED,
    STATE_READY,
    STATE_PREEMPT
} process_state_t;

//Part of Creating the classes

class Process{
    public:
        int pid;
        int AT;
        int TC;
        int CB;
        int IO;
        int PRIO;
        process_state_t state;
        int state_ts = 0;
        int TS; //use for saving the next timestamp for the process

        int FT,TT,IT,CW = 0;
        
        int cb;
        int ib;
        int rem;
        int prio;

        bool been_prem = false;
        bool be_pr = false;

    Process(int pid, int AT, int TC, int CB, int IO, int PRIO){
        (*this).pid = pid;
        (*this).AT = AT;
        (*this).TC = TC;
        (*this).CB = CB;
        (*this).IO = IO;
        (*this).PRIO = PRIO;
        //prio = PRIO-1;
    }
};

Process* CURRENT_RUNNING_PROCESS;
//output trigger bool
bool v_printer = false;
bool t_printer = false;
bool e_printer = false;

class Scheduler{
    public :
        virtual void add_process(Process *p) = 0;
        virtual Process* get_next_process() = 0;
        virtual void test_preempt(Process *p, int curtime) = 0; // typically NULL but for 'E'
};

class FCFS_Scheduler: public Scheduler{
    public :
        queue<Process*> RUN_QUEUE;

        void add_process(Process *p){
            RUN_QUEUE.push(p);
        };

        Process* get_next_process(){
            if(RUN_QUEUE.size() > 0){
                Process * temp_process = RUN_QUEUE.front();
                RUN_QUEUE.pop();
                return temp_process;    
            }
            return nullptr;
        };
        
        void test_preempt(Process *p, int curtime){};
};

class LCFS_Scheduler: public Scheduler{
    public: 
        stack<Process*> RUN_QUEUE;

        void add_process(Process *p){
            RUN_QUEUE.push(p);
        };

        Process* get_next_process(){
            if(RUN_QUEUE.size() > 0){
                Process * temp_process = RUN_QUEUE.top();
                RUN_QUEUE.pop();
                return temp_process;
            }
            return nullptr;
        };

        void test_preempt(Process *p, int curtime){};

};

class SRTF_Scheduler: public Scheduler{
    public :
        list<Process*> RUN_QUEUE;

        void add_process(Process *p){
            list<Process*>::iterator ITER;
            for(ITER = RUN_QUEUE.begin(); ITER != RUN_QUEUE.end(); ++ITER){
                Process* temp_process = (Process*)(*ITER);
                if(p->rem < temp_process->rem){
                    break;
                }
            }
            RUN_QUEUE.insert(ITER,p);
        };

        Process* get_next_process(){
            if(RUN_QUEUE.size() > 0){
                Process * temp_process = RUN_QUEUE.front();
                RUN_QUEUE.pop_front();
                return temp_process;
            }
            return nullptr;
        };
        
        void test_preempt(Process *p, int curtime){};

};

class RR_Scheduler: public Scheduler{
    public:
        queue<Process*> RUN_QUEUE;

        void add_process(Process *p){
            RUN_QUEUE.push(p);
        };

        Process* get_next_process(){
            if(RUN_QUEUE.size() > 0){
                Process * temp_process = RUN_QUEUE.front();
                RUN_QUEUE.pop();
                return temp_process;    
            }
            return nullptr;
        };
        
        void test_preempt(Process *p, int curtime){};
};

class PRIO_Scheduler: public Scheduler{
    public:
        
        queue<Process*> *activeQ;
        queue<Process*> *expiredQ;

        int maxprio;

        PRIO_Scheduler(int maxprio){
            activeQ = new queue<Process*>[maxprio]();
            expiredQ = new queue<Process*>[maxprio]();
            this->maxprio = maxprio;
        }

        int find_first_bit(){
            for(int i = maxprio-1; i > -1; i--){
                if(activeQ[i].size() > 0){
                    return i;
                }
            }
            return -1;
        };

        void add_process(Process *p){
            if(p->prio < 0){
                p->prio = p->PRIO - 1;
                //cout << "push " << p->pid << " in expired" << endl; 
                expiredQ[p->prio].push(p);
            }
            else{
                //cout << "push " << p->pid << " in active" << endl; 
                activeQ[p->prio].push(p);
            }
        };

        Process* get_next_process(){
            int max_bit = find_first_bit();
            //cout <<"MAX: "<<max_bit<<endl;
            if(activeQ[max_bit].size() > 0 && max_bit != -1){
                Process * temp_process = activeQ[max_bit].front();
                //cout << "pop " << temp_process->pid << " in active" << endl;
                activeQ[max_bit].pop();
                return temp_process;
            }else{
                //cout <<"exchange"<<endl;
                queue<Process*> *tempQ;
                tempQ = activeQ;
                activeQ = expiredQ;
                expiredQ = tempQ;
                max_bit = find_first_bit();
                //cout <<"MAX: "<<max_bit<<endl;
                if(activeQ[max_bit].size() > 0 && max_bit != -1){
                    Process * temp_process = activeQ[max_bit].front();
                    //cout << "pop " << temp_process->pid << " in active" << endl;
                    activeQ[max_bit].pop();
                    return temp_process;
                }else{
                    return nullptr;
                }
            }
            return nullptr;
        };

        
        

        void test_preempt(Process *p, int curtime){};
};

class E_Scheduler: public Scheduler{
    public:
        
        queue<Process*> *activeQ;
        queue<Process*> *expiredQ;

        int maxprio;

        E_Scheduler(int maxprio){
            activeQ = new queue<Process*>[maxprio]();
            expiredQ = new queue<Process*>[maxprio]();
            this->maxprio = maxprio;
        }

        int find_first_bit(){
            for(int i = maxprio-1; i > -1; i--){
                if(activeQ[i].size() > 0){
                    return i;
                }
            }
            return -1;
        };

        void add_process(Process *p){
            if(p->prio < 0){
                p->prio = p->PRIO - 1;
                //cout << "push " << p->pid << " in expired" << endl; 
                expiredQ[p->prio].push(p);
            }
            else{
                //cout << "push " << p->pid << " in active" << endl; 
                activeQ[p->prio].push(p);
            }
        };

        Process* get_next_process(){
            int max_bit = find_first_bit();
            //cout <<"MAX: "<<max_bit<<endl;
            if(activeQ[max_bit].size() > 0 && max_bit != -1){
                Process * temp_process = activeQ[max_bit].front();
                //cout << "pop " << temp_process->pid << " in active" << endl;
                activeQ[max_bit].pop();
                return temp_process;
            }else{
                //cout <<"exchange"<<endl;
                queue<Process*> *tempQ;
                tempQ = activeQ;
                activeQ = expiredQ;
                expiredQ = tempQ;
                max_bit = find_first_bit();
                //cout <<"MAX: "<<max_bit<<endl;
                if(activeQ[max_bit].size() > 0 && max_bit != -1){
                    Process * temp_process = activeQ[max_bit].front();
                    //cout << "pop " << temp_process->pid << " in active" << endl;
                    activeQ[max_bit].pop();
                    return temp_process;
                }else{
                    return nullptr;
                }
            }
            return nullptr;
        };

        
        

        void test_preempt(Process *p, int curtime){
            if(CURRENT_RUNNING_PROCESS == nullptr){
                return;
            }
            // int max_bit = find_first_bit();
            // Process * temp_process = activeQ[max_bit].front();
            if(p->prio > CURRENT_RUNNING_PROCESS->prio){
                if(CURRENT_RUNNING_PROCESS->TS > curtime){
                    if(v_printer){
                        cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS->pid <<" by " << p->pid
                        << " ? 1 TS=" << CURRENT_RUNNING_PROCESS->TS << " now=" << curtime <<") --> YES" << endl; 
                        CURRENT_RUNNING_PROCESS->be_pr = true;
                    }

                }else{
                    if(v_printer){
                        cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS->pid <<" by " << p->pid
                        << " ? 1 TS=" << CURRENT_RUNNING_PROCESS->TS << " now=" << curtime <<") --> NO" << endl; 
                    }
                }
            }else{
                if(v_printer){
                    cout << "---> PRIO preemption " << CURRENT_RUNNING_PROCESS->pid <<" by " << p->pid
                    << " ? 0 TS=" << CURRENT_RUNNING_PROCESS->TS << " now=" << curtime <<") --> NO" << endl; 
                }

            }
        };
};


class EVENT{
    public:
        Process * evtProcess;
        int evtTimeStamp;
        process_state_t oldstate;
        process_state_t newstate;
        EVENT(){};

        EVENT(Process * evtProcess, int evtTimeStamp, process_state_t oldstate, process_state_t newstate){
            this->evtProcess = evtProcess;
            this->evtTimeStamp = evtTimeStamp;
            this->oldstate = oldstate;
            this->newstate = newstate;
        };

        //PRINTER OF EVENT
        // void toString(){
        //     cout << evtTimeStamp << ' ' << evtProcess->pid << ' ' <<  endl;
        // }


};

// summary var
int finishing_time;
double cpu_util;
double io_util;
double avg_turnaround_time;
double avg_cpu_waiting_time;
double num_of_throughput;

//random var
vector<int> random_num_array;
int size_of_random_array = 0;
int ofs = 0;

//simulator var
bool CALL_SCHEDULER;
int timeInPrevState;
int CURRENT_TIME = 0;

Scheduler *THE_SCHEDULER;




//set a default quantum for those non-RR schedulars, a default prio for those non_p schedulars
int quantum = 10000;
int maxprios = 4;

//initialize the event list and process list
list <EVENT> EVENT_LIST;
list <Process*> PROCESS_LIST;

//some var for storing the summary information
double total_program_num;
double total_cpu_time;
double total_io_time;
double total_turnaround_time;
double total_cpu_waiting_time;
int io_usage; //this var is to consider wheather there is already some proc in IO
int io_start;

//bool for pree
bool isPREE= false;

void create_event(Process * evtProcess, int evtTimeStamp, process_state_t oldstate, process_state_t newstate);

//Part of activing the random function



void loading_random_num_file(char* filename){
    ifstream infile;
    infile.open(filename);
    string temp_str;
    if(infile.is_open()){
        getline(infile,temp_str);
        size_of_random_array = atoi(temp_str.c_str());
        //random_num_array = new int[size_of_random_array]();
        while(getline(infile,temp_str)){
            //cout << temp_str << endl;
            random_num_array.push_back(atoi(temp_str.c_str()));
        } 
    }
    // int temp_array[size_of_random_array];
    // int index = 0;
    // while(getline(infile,temp_str)){
    //         //cout << temp_str << endl;
    //         temp_array[index] = atoi(temp_str.c_str());
    //         index ++;
    //     } 
    // for(int i = 0; i < size_of_random_array; i++){
    //     cout << temp_array[i] << endl;
    // }
    infile.close();
}
int myrandom(int burst){
    int temp_return_val = 1 + (random_num_array[ofs] % burst);
    ofs ++;
    if(ofs == size_of_random_array){
        ofs = 0;
    }
    return temp_return_val;
}

// initilize the input file
void load_input_file(char* filename){
    ifstream infile;
    infile.open(filename);
    string line,number;
    int pid = 0;
    int temparray[4];
    if(infile.is_open()){
        while(!infile.eof()){
            for(int i =0; i <4; i++){
                infile >> number;
                temparray[i] = atoi(number.c_str());
            }
            if(infile.eof()){
                break;
            }
            int prio = myrandom(maxprios);
            //cout << prio << endl;
            
            Process *new_proc = new Process(pid, temparray[0],temparray[1], temparray[2], temparray[3], prio);   
            PROCESS_LIST.push_back(new_proc);
            new_proc->prio = prio-1;
            new_proc->state_ts = 0;
            new_proc->rem = new_proc->TC;
            new_proc->TS = new_proc->AT;
            create_event(new_proc, new_proc->AT, STATE_CREATED, STATE_READY);
            pid++;
        }
    }
    total_program_num = pid;
    infile.close();
}


EVENT * get_event(){
    if(EVENT_LIST.size() >0){
        return &EVENT_LIST.front();
    }
    return NULL;
}



int get_next_event_time(){
    if(EVENT_LIST.size() > 0){
        return EVENT_LIST.front().evtTimeStamp;
    }
    return -1;
}

void create_event(Process * evtProcess, int evtTimeStamp, process_state_t oldstate, process_state_t newstate){
    EVENT new_event(evtProcess, evtTimeStamp, oldstate, newstate);

    if(EVENT_LIST.size() <= 0){
        EVENT_LIST.push_back(new_event);
        // cout << " details of proc in evt" << endl;
        // cout << new_event.evtProcess->state_ts << endl;
        // cout << new_event.evtProcess->AT << endl;
    }
    else{
        // cout << " details of proc in evt" << endl;
        // cout << new_event.evtProcess->state_ts << endl;
        // cout << new_event.evtProcess->AT << endl;
        list<EVENT>::iterator ITER;
        for(ITER = EVENT_LIST.begin(); ITER != EVENT_LIST.end(); ++ITER){
            if(new_event.evtTimeStamp < ITER->evtTimeStamp){
                break;
            }
        }
        EVENT_LIST.insert(ITER,new_event);
    }
    // cout << "evt list" << endl;
    // list<EVENT>::iterator ITER;
    // for(ITER = EVENT_LIST.begin(); ITER != EVENT_LIST.end(); ITER++){
    //     cout << ITER ->evtProcess->pid << " ";
    //     cout << ITER ->evtTimeStamp << " ";
    //     cout << ITER ->evtProcess->state_ts << " ";
    //     cout << ITER ->oldstate << " ";
    //     cout << ITER ->newstate << endl;
    // }
    
}

void rm_event(){
    EVENT_LIST.erase(EVENT_LIST.begin());
}

void Simulation() {
    EVENT* evt;
    while( (evt = get_event()) ) {
        Process *proc = evt->evtProcess; // this is the process the event works on
        //cout << proc->pid << " " << proc->AT << " " << proc->state_ts << endl;
        CURRENT_TIME = evt->evtTimeStamp;
        timeInPrevState = CURRENT_TIME-proc->state_ts;
        proc->state_ts = CURRENT_TIME;
        switch(evt->newstate) { // which state to transition to?
            case STATE_READY:{
                // must come from BLOCKED or from PREEMPTION or from CREATED
                // must add to run queue
                if(evt->oldstate == STATE_CREATED){
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": CREATED -> READY" << endl;
                    }
                }
                else if(evt->oldstate == STATE_BLOCKED){
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": BLOCK -> READY" << endl;
                    }   
                    io_usage--;
                    if(io_usage == 0){
                        total_io_time += (CURRENT_TIME - io_start);
                    }
                }
                if(CURRENT_RUNNING_PROCESS != nullptr){
                    THE_SCHEDULER->test_preempt(proc,CURRENT_TIME); 
                    if(CURRENT_RUNNING_PROCESS->be_pr){
                        if(CURRENT_RUNNING_PROCESS->been_prem){
                            CURRENT_RUNNING_PROCESS->cb += quantum;
                            CURRENT_RUNNING_PROCESS->rem += quantum;
                        }
                        CURRENT_RUNNING_PROCESS->cb -= CURRENT_TIME - CURRENT_RUNNING_PROCESS->state_ts;
                        CURRENT_RUNNING_PROCESS->rem -= CURRENT_TIME - CURRENT_RUNNING_PROCESS->state_ts;
                        CURRENT_RUNNING_PROCESS->TS = CURRENT_TIME;
                        list<EVENT>::iterator ITER;
                        for(ITER = EVENT_LIST.begin(); ITER != EVENT_LIST.end(); ++ITER){
                            Process* temp_process = ITER->evtProcess;
                            if(CURRENT_RUNNING_PROCESS->pid == temp_process->pid){
                                break;
                            }
                        }
                        EVENT_LIST.erase(ITER);
                        create_event(CURRENT_RUNNING_PROCESS, CURRENT_TIME, STATE_RUNNING, STATE_PREEMPT);
                        CURRENT_RUNNING_PROCESS->be_pr = false;
                        CURRENT_RUNNING_PROCESS->been_prem = true;
                    }

                }
                // if(isPREE){
                //     THE_SCHEDULER->test_preempt(proc,CURRENT_TIME); 
                //     if(CURRENT_RUNNING_PROCESS != nullptr){
                //         if(CURRENT_RUNNING_PROCESS->be_pr){
                //             CURRENT_RUNNING_PROCESS->cb -= timeInPrevState;
                //             CURRENT_RUNNING_PROCESS->TS = CURRENT_TIME;
                //             create_event(CURRENT_RUNNING_PROCESS, CURRENT_TIME, STATE_RUNNING, STATE_PREEMPT);
                            
                //             list<EVENT>::iterator ITER;
                //             for(ITER = EVENT_LIST.begin(); ITER != EVENT_LIST.end(); ++ITER){
                //                 Process* temp_process = ITER->evtProcess;
                //                 if(CURRENT_RUNNING_PROCESS->pid == temp_process->pid){
                //                     break;
                //                 }
                //             }
                //             EVENT_LIST.erase(ITER);
                //             CURRENT_RUNNING_PROCESS->be_pr = false;
                //         }
                //     }

                // }
                proc->state = STATE_READY;
                THE_SCHEDULER->add_process(proc);
                //cout << "add into" << endl;
                CALL_SCHEDULER = true; // conditional on whether something is run
            }break;
            case STATE_RUNNING:{
                if(!proc->been_prem){
                    proc->cb = myrandom(proc->CB);
                    if(proc->cb > proc->rem){
                        proc->cb = proc->rem;
                    }
                }

                if(proc->cb <=quantum){
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": READY -> RUNNG" << " cb=" << proc->cb << " rem=" << proc->rem << " prio=" << proc->prio<< endl;
                    } 
                    proc->been_prem = false;
                    proc->TS = CURRENT_TIME+proc->cb;
                    create_event(proc,CURRENT_TIME+proc->cb,STATE_RUNNING,STATE_BLOCKED);     
                }
                else{
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": READY -> RUNNG" << " cb=" << proc->cb << " rem=" << proc->rem << " prio=" << proc->prio<< endl;
                    } 

                    //cout << "cb before:" << proc->cb << endl;
                    proc->cb -= quantum;
                    //cout << "cb after:" << proc->cb << endl;
                    proc->rem -= quantum;
                    proc->TS = CURRENT_TIME+quantum;
                    create_event(proc, CURRENT_TIME+quantum, STATE_RUNNING, STATE_PREEMPT);
                    proc->been_prem = true;
                }
                // create event for either preemption or blocking
                proc->CW += timeInPrevState;
                proc->state = STATE_RUNNING;
            }break;

            case STATE_BLOCKED:{
                CURRENT_RUNNING_PROCESS = nullptr;
                proc->rem -= proc->cb;
                if(proc->rem == 0){
                    proc->FT = CURRENT_TIME;
                    proc->TT = proc->FT-proc->AT;
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": Done" << endl;
                    } 
                }
                else{
                    proc->ib = myrandom(proc->IO);
                    proc->IT += proc->ib;
                    if(v_printer){
                        cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": RUNNG -> BLOCK" << " ib=" << proc->ib << " rem=" << proc->rem << endl;
                    } 
                    //proc->TC = CURRENT_TIME+proc->ib;
                    create_event(proc, CURRENT_TIME+proc->ib, STATE_BLOCKED, STATE_READY);
                    proc->state = STATE_BLOCKED;
                    if(io_usage == 0){
                        io_start = CURRENT_TIME;
                    }
                    io_usage++;
                    proc->prio = proc->PRIO-1;
                    
                }
                //create an event for when process becomes READY again     
                CALL_SCHEDULER = true;
            }break;
            case STATE_PREEMPT:{
                // add to runqueue (no event is generated)
                CURRENT_RUNNING_PROCESS = nullptr;
                //proc->rem -= quantum;
                if(v_printer){
                    cout << CURRENT_TIME << ' ' << proc->pid << ' ' << timeInPrevState << ": RUNNG -> READY" << " cb=" << proc->cb << " rem=" << proc->rem << " prio=" << proc->prio << endl;
                } 
                proc->prio--;
                THE_SCHEDULER->add_process(proc);
                CALL_SCHEDULER = true;
            }break;
        }
        
        // remove current event object from Memory
        rm_event();
        
        //delete evt; 
        evt = nullptr;
        
        if(CALL_SCHEDULER) {
            //cout << " in sche" << endl;
            if (get_next_event_time() == CURRENT_TIME)
                continue; //process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr){
                CURRENT_RUNNING_PROCESS = THE_SCHEDULER->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                    continue;
                else{
                    CURRENT_RUNNING_PROCESS->TS = CURRENT_TIME;
                    create_event(CURRENT_RUNNING_PROCESS, CURRENT_TIME, STATE_READY, STATE_RUNNING);
                }
                // create event to make this process runnable for same time.
            }  
            else{
                continue;
            } 
        } 
    } 
    // cout << quantum << endl;
    // cout << maxprios << endl;
    // cout << "End simulation" << endl;
}



int main(int argc, char* argv[]){
    // if (argc != 2 ){
    //     cout << "Error: file input format wrong" << endl;
    //     exit(0);
    // }
    char* inputFileName;
    char* randomFileName;
    //loading_random_num_file(randomFileName);
    // for(int i = 0; i < size_of_random_array; i++){
    //         cout <<random_num_array[i]<< endl;
    // }
    int c;
    int index;
    string schedspec;
    while((c = getopt (argc, argv, "vtes:")) != -1)
        switch(c)
            {
            case 'v':
                v_printer = true;
                //cout << "get v" << endl;
                break;
            case 't':
                t_printer = true;
                //cout << "get t" << endl;
                break;
            case 'e':
                e_printer = true;
                //cout << "get e" << endl;
                break;
            case 's':
                schedspec = optarg;
                //sscanf(optarg, "%[A-Z]%d:%d", &quantum, &maxprios);
                break;
            default:
                break;
            }

            for (index = optind; index < argc; index++){
                if (index==argc-2){
                    //cout << argv[index] << endl;
                    inputFileName = argv[index];
                }
                if (index==argc-1){
                    //cout << argv[index] << endl;
                    randomFileName = argv[index];
                }
            }

    string title;
    //choose the right schedular type
    if(schedspec.at(0) == 'F'){
        THE_SCHEDULER = new FCFS_Scheduler();
        title = "FCFS";
    }
    else if(schedspec.at(0) == 'L'){
        THE_SCHEDULER = new LCFS_Scheduler();
        title = "LCFS";
    }
    else if(schedspec.at(0) == 'S'){
        THE_SCHEDULER = new SRTF_Scheduler();
        title = "SRTF";
    }
    else if(schedspec.at(0) == 'R'){
        THE_SCHEDULER = new RR_Scheduler();
        quantum = atoi(schedspec.substr(1,schedspec.size()-1).c_str());
        title = "RR "+ schedspec.substr(1,schedspec.size()-1);
    }
    else if(schedspec.at(0) == 'P'){
        bool has_quote = false;
        string quant;
        for(int i = 1; i < schedspec.size(); i++){
            if(schedspec.at(i) == ':'){
                has_quote = true;
                quant = schedspec.substr(1,i-1);
                quantum = atoi(schedspec.substr(1,i-1).c_str());
                maxprios = atoi(schedspec.substr(i+1,schedspec.size()).c_str());
                break;
            }

        }
        if(!has_quote){
            quant = schedspec.substr(1,schedspec.size()-1);
            quantum = atoi(schedspec.substr(1,schedspec.size()-1).c_str());
        }
        THE_SCHEDULER = new PRIO_Scheduler(maxprios);
        // cout << "Quantum: " << quantum << endl;
        // cout << "Maxprios: " << maxprios << endl;
        title = "PRIO "+ quant;
    }
    else if(schedspec.at(0) == 'E'){
        bool has_quote = false;
        string quant;
        for(int i = 1; i < schedspec.size(); i++){
            if(schedspec.at(i) == ':'){
                has_quote = true;
                quant = schedspec.substr(1,i-1);
                quantum = atoi(schedspec.substr(1,i-1).c_str());
                maxprios = atoi(schedspec.substr(i+1,schedspec.size()).c_str());
                break;
            }

        }
        if(!has_quote){
            quant = schedspec.substr(1,schedspec.size()-1);
            quantum = atoi(schedspec.substr(1,schedspec.size()-1).c_str());
        }
        THE_SCHEDULER = new E_Scheduler(maxprios);
        isPREE = true;
        
        title = "PREPRIO "+ quant;
    }

    loading_random_num_file(randomFileName);
    load_input_file(inputFileName);
    
    Simulation();
    cout << title << endl;
    Process * p;
    while(PROCESS_LIST.size() > 0){
        p = PROCESS_LIST.front();
        cout << setw(4) << setfill('0') << p ->pid;
        cout << ": " << setw(4) << setfill(' ') << p->AT << ' ' << setw(4) << p->TC
        <<" "<< setw(4) << p->CB <<" "<< setw(4) << p->IO <<" "<< setw(1) << p->PRIO
        << " | " << setw(5) << p->FT <<" "<< setw(5) << p->TT <<" "<< setw(5) << p->IT
        <<" "<< setw(5) << p->CW << endl;
        total_cpu_time += p->TC;
        total_turnaround_time += p->TT;
        total_cpu_waiting_time += p->CW;
        PROCESS_LIST.pop_front();

    }
    double finishing_time = CURRENT_TIME;
    cout << "SUM: " << CURRENT_TIME << " " << setiosflags(ios::fixed)<<setprecision(2) << total_cpu_time/finishing_time*100.0 
    << " " << total_io_time/finishing_time*100.0 << " " << total_turnaround_time/total_program_num << " " << total_cpu_waiting_time/total_program_num;
    cout << setiosflags(ios::fixed) << setprecision(3) << " " << total_program_num*100.0/finishing_time;
}