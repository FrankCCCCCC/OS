#include <stack>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <queue>
#include <vector>
#include <unistd.h>

using namespace std;


bool v_trigger = false;
bool q_trigger = false;
bool f_trigger = false;

class io_t {
    public:
        int op;
        int arrival_time;
        int track;
        int start_time;
        int end_time;

        io_t(){};

        io_t(int op, int arrival_time, int track){
        this->op = op;
        this->arrival_time = arrival_time;
        this->track = track;
        }
};

vector<io_t> io_vector;
int last_idx = 0;
int sys_time = 1;
int curr_track = 0;


class IO_Scheduler{
    public:
        virtual void add_to_ready(io_t* temp_io) = 0;
        virtual io_t* get_next_request() = 0;
        
};

class FIFO_Schedulaer: public IO_Scheduler{
    public:
        queue <io_t*> ready_queue;

        void add_to_ready(io_t* temp_io){
            ready_queue.push(temp_io);
        };

        io_t* get_next_request(){
            if(ready_queue.size() > 0){
                io_t *next_io = ready_queue.front();
                ready_queue.pop();
                return next_io;
            }
            else{
                return nullptr;
            }
        };
        
};

class SSTF_Schedulaer: public IO_Scheduler{
    public:
        vector <io_t*> ready_queue;

        void add_to_ready(io_t* temp_io){
            ready_queue.push_back(temp_io);
        };

        io_t* get_next_request(){
            if(ready_queue.size() > 0){

                int opt_idx = 0;
                int opt_dist = abs(ready_queue[0]->track - curr_track);
                for(int i=1; i < ready_queue.size(); i++){
                    int temp_dist = abs(ready_queue[i]->track - curr_track);
                    if(temp_dist < opt_dist){
                        opt_dist = temp_dist;
                        opt_idx = i;
                    }
                }
                io_t *next_io = ready_queue[opt_idx];     
                //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                ready_queue.erase(ready_queue.begin()+opt_idx); 
                return next_io;
            }
            else{
                return nullptr;
            }
        };
        
};

class LOOK_Schedulaer: public IO_Scheduler{
    public:
        vector <io_t*> ready_queue;
        bool forward_trigger = true;

        void add_to_ready(io_t* temp_io){
            ready_queue.push_back(temp_io);
        };

        io_t* get_next_request(){
            if(ready_queue.size() > 0){
                // check wheather there is a turn around here 
                // and we can cathc new io with different direction
                if(forward_trigger){
                    int max_track = ready_queue[0]->track; 
                    for(int i = 1; i < ready_queue.size(); i++){
                        if(ready_queue[i]->track >= max_track){
                            max_track = ready_queue[i]->track;
                        }
                    }
                    if(max_track < curr_track){
                        forward_trigger = false;
                    }
                }else{
                    int min_track = ready_queue[0]->track;
                    for(int i = 1; i < ready_queue.size(); i++){
                        if(ready_queue[i]->track <= min_track){
                            min_track = ready_queue[i]->track;
                        }
                    }
                    if(min_track > curr_track){
                        forward_trigger = true;
                    }     
                }
                
                // choose the new io
                if(forward_trigger){
                    int opt_idx = 0;
                    int opt_dist = 1024;
                    for(int i=0; i < ready_queue.size(); i++){
                        int temp_dist = ready_queue[i]->track - curr_track;
                        if(temp_dist >=0 && temp_dist < opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = ready_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    ready_queue.erase(ready_queue.begin()+opt_idx); 
                    return next_io;
                }else{
                    int opt_idx = 0;
                    int opt_dist =  1024;
                    for(int i=0; i < ready_queue.size(); i++){
                        int temp_dist = curr_track - ready_queue[i]->track;
                        if(temp_dist >=0 && temp_dist < opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = ready_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    ready_queue.erase(ready_queue.begin()+opt_idx); 
                    return next_io;
                }
            }
            else{
                return nullptr;
            }
        };
        
};

class CLOOK_Schedulaer: public IO_Scheduler{
    public:
        vector <io_t*> ready_queue;
        bool forward_trigger = true;

        void add_to_ready(io_t* temp_io){
            ready_queue.push_back(temp_io);
        };

        io_t* get_next_request(){
            if(ready_queue.size() > 0){
                
                // check wheather there is a restart here 
                // and we can catch new io with longest distance
                int max_track = ready_queue[0]->track; 
                for(int i = 1; i < ready_queue.size(); i++){
                    if(ready_queue[i]->track >= max_track){
                        max_track = ready_queue[i]->track;
                    }
                }
                if(max_track < curr_track){
                    forward_trigger = false;
                }
                
                // choose the new io
                if(forward_trigger){
                    int opt_idx = 0;
                    int opt_dist = 1024;
                    for(int i=0; i < ready_queue.size(); i++){
                        int temp_dist = ready_queue[i]->track - curr_track;
                        if(temp_dist >=0 && temp_dist < opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = ready_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    ready_queue.erase(ready_queue.begin()+opt_idx); 
                    return next_io;
                }else{
                    int opt_idx = 0;
                    int opt_dist = 0;
                    for(int i=0; i < ready_queue.size(); i++){
                        int temp_dist = curr_track - ready_queue[i]->track;
                        if(temp_dist >=0 && temp_dist > opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = ready_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    ready_queue.erase(ready_queue.begin()+opt_idx); 
                    forward_trigger = true;
                    return next_io;
                }
            }
            else{
                return nullptr;
            }
        };
        
};

class FLOOK_Schedulaer: public IO_Scheduler{
    public:
        vector <io_t*> active_queue;
        vector <io_t*> add_queue;
        bool forward_trigger = true;

        void add_to_ready(io_t* temp_io){
            add_queue.push_back(temp_io);
            // cout << "Situation in add queue" << endl;
            // for(int i = 0; i<add_queue.size(); i++){
            //     cout << add_queue[i]->op << " " << add_queue[i]->arrival_time << endl;

            // }
        };

        io_t* get_next_request(){
            if(active_queue.size() <= 0){
                active_queue.swap(add_queue);
                // cout << "SWAP HAPPEN" << endl;
            }


            if(active_queue.size() > 0){
                // check wheather there is a turn around here 
                // and we can cathc new io with different direction
                if(forward_trigger){
                    int max_track = active_queue[0]->track; 
                    for(int i = 1; i < active_queue.size(); i++){
                        if(active_queue[i]->track >= max_track){
                            max_track = active_queue[i]->track;
                        }
                    }
                    if(max_track < curr_track){
                        forward_trigger = false;
                    }
                }else{
                    int min_track = active_queue[0]->track;
                    for(int i = 1; i < active_queue.size(); i++){
                        if(active_queue[i]->track <= min_track){
                            min_track = active_queue[i]->track;
                        }
                    }
                    if(min_track > curr_track){
                        forward_trigger = true;
                    }     
                }
                
                // choose the new io
                if(forward_trigger){
                    int opt_idx = 0;
                    int opt_dist = 1024;
                    for(int i=0; i < active_queue.size(); i++){
                        int temp_dist = active_queue[i]->track - curr_track;
                        if(temp_dist >=0 && temp_dist < opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = active_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    active_queue.erase(active_queue.begin()+opt_idx); 
                    return next_io;
                }else{
                    int opt_idx = 0;
                    int opt_dist =  1024;
                    for(int i=0; i < active_queue.size(); i++){
                        int temp_dist = curr_track - active_queue[i]->track;
                        if(temp_dist >=0 && temp_dist < opt_dist){
                            opt_dist = temp_dist;
                            opt_idx = i;
                        }
                    }
                    io_t *next_io = active_queue[opt_idx];     
                    //std::vector<io_t*>::iterator it = ready_queue.begin()+opt_idx; 
                    active_queue.erase(active_queue.begin()+opt_idx); 
                    return next_io;
                }
            }
            else{
                
                return nullptr;
            }
        };
        
};


IO_Scheduler *THE_SCHEDULER;

void load_input_file(char* inputfileName){
    ifstream infile(inputfileName);
    string line;
    int pro_num = 0;
    int vma_num = 0;
    int op = 0;
    if(infile.is_open()){
        while (getline(infile, line)){
            if(line.find("#") != string::npos){
                continue;
            }
            istringstream is(line);
            io_t new_io = io_t();
            is >> new_io.arrival_time >> new_io.track;
            new_io.op = op;
            io_vector.push_back(new_io);
            op++;
        }
    }
    infile.close();
}



int total_movement = 0;

void Simulation(){
    //io_t *pending_io;
    io_t *running_io;
    bool active_trigger = true;
    int step_size = 1;
    int finish_counter = 0;


    while(true){
        
        //cout << "systime: "  << sys_time<< endl;
        //check for any new request
        io_t *new_ready;
        for(int i=last_idx; i < io_vector.size(); i++){
            new_ready = &io_vector[i];
            if(new_ready->arrival_time == sys_time){
                //cout << sys_time << ": "<< i << " add " << new_ready->track << endl;
                THE_SCHEDULER->add_to_ready(new_ready);
                last_idx++;
            }else if(new_ready->arrival_time > sys_time){
                break;
            }
        }
        
        // initalize the first requset into active
        if(active_trigger){
            running_io = THE_SCHEDULER->get_next_request();
            if(running_io == nullptr){
                if(finish_counter == io_vector.size()){
                    break;
                }
                sys_time++;
                active_trigger = true;
                continue;
                
            }
            running_io->start_time = sys_time;
            active_trigger = false;
        }

        if(running_io->track > curr_track){
            step_size = 1;
        }else if(running_io->track < curr_track){
            step_size = -1;
        }


        
        if(curr_track == running_io->track){

            //cout << sys_time << ": "  << running_io->op << " finish" << endl;
            //save the relevant info
            running_io->end_time = sys_time;
            finish_counter++;
            

            //no io_request active, ask for next request
            running_io = THE_SCHEDULER->get_next_request();
            
            //if no request pending, exit 
            if(running_io == nullptr){
                
                if(finish_counter == io_vector.size()){
                    break;
                }
                sys_time++;
                active_trigger = true;
                continue;
                
            }

            //if the current track match the new request, 
            // save the info and ask for a new one
            while(running_io->track == curr_track){
                // cout << sys_time << ": "  << running_io->op << " issue " << running_io->track << " " << curr_track << endl;
                running_io->start_time = sys_time;
                // cout << sys_time << ": "  << running_io->op << " finish" << endl;
                finish_counter++;
                running_io->end_time = sys_time;
                //cout << "go to here" << endl;
                running_io = THE_SCHEDULER->get_next_request();
                if(running_io == nullptr){
                        break;
                }
            }
            if(running_io == nullptr){
                if(finish_counter ==io_vector.size()){
                    break;
                }
                sys_time++;
                active_trigger = true;
                continue;
            }

            //save the relevant info for start time
            running_io->start_time = sys_time;


            //cout << "check the direction" << running_io->track <<" : " << curr_track << endl;
            //check the direction for seeking
            if(running_io->track > curr_track){
                step_size = 1;
            }else if(running_io->track < curr_track){
                step_size = -1;
            }
            //move one step
            curr_track += step_size;
            total_movement++;
        }
        else{
            //cout << sys_time << ": "  << running_io->op << " issue " << running_io->track << " " << curr_track << endl;
            //there is an active request, just move one step;
            curr_track += step_size;
            total_movement++;
        }

        //Increment time by 1
        sys_time++;

    }
    for(int i = 0; i<io_vector.size(); i++){
        //cout << io_vector[i].op << " " << io_vector[i].arrival_time << endl;
        //printf("%5d: %5d\n", i, io_vector[i].op);
        printf("%5d: %5d %5d %5d\n",i, io_vector[i].arrival_time, io_vector[i].start_time, io_vector[i].end_time);
    }
    //SUM info
    int total_time = sys_time;
    double total_turnaround = 0.0;
    double total_waittime = 0.0;
    int max_waittime = 0;
    for(int i = 0; i<io_vector.size(); i++){
        //cout << io_vector[i].end_time-io_vector[i].start_time << endl;
        total_turnaround += (io_vector[i].end_time-io_vector[i].arrival_time);
        //cout << total_turnaround << endl;
        total_waittime += (io_vector[i].start_time-io_vector[i].arrival_time);
        if((io_vector[i].start_time-io_vector[i].arrival_time) > max_waittime){
            max_waittime = (io_vector[i].start_time-io_vector[i].arrival_time);
        }
    }
    //cout << total_turnaround/io_vector.size() << endl;
    printf("SUM: %d %d %.2lf %.2lf %d\n",total_time, total_movement, total_turnaround/io_vector.size(), total_waittime/io_vector.size(), max_waittime);
};

int main(int argc, char* argv[]){

    char* inputFileName;

    int c;
    int index;
    string io_algo;
    string output;

    while((c = getopt (argc, argv, "s:v:q:f")) != -1)
        switch(c)
            {
            case 's':
                io_algo = optarg;
                break;
            case 'v':
                v_trigger = true;
                break;
            case 'q':
                break;
            case 'f':
                break;
            default:
                break;
            }

            for (index = optind; index < argc; index++){
                if (index==argc-1){
                    //cout << argv[index] << endl;
                    inputFileName = argv[index];
                }
            }



    if(io_algo.at(0) == 'i'){
        THE_SCHEDULER = new FIFO_Schedulaer();
        //cout << "get f pager" << endl;
    }
    else if(io_algo.at(0) == 'j'){
        THE_SCHEDULER = new SSTF_Schedulaer();
    }
    else if(io_algo.at(0) == 's'){
        THE_SCHEDULER = new LOOK_Schedulaer();
    }
    else if(io_algo.at(0) == 'c'){
        THE_SCHEDULER = new CLOOK_Schedulaer();
    }
    else if(io_algo.at(0) == 'f'){
        THE_SCHEDULER = new FLOOK_Schedulaer();
    }

    load_input_file(inputFileName);

    Simulation();

    //cout << io_algo << endl;
}