#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include<stdio.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <queue>
#include <stack>
#include <algorithm>
#include <getopt.h>
#include <climits>
#include <list>
#include <map>

#define MAX_VPAGE 64

using namespace std;


typedef struct {
    unsigned present     :1;
    unsigned referenced  :1;
    unsigned modified    :1;
    unsigned write_protect :1;
    unsigned pagedout    :1;
    unsigned frameIndex  :7;
    unsigned filemapped :1;
    unsigned non_hole :1;
    unsigned :18;
} pte_t;

typedef struct {
    int pid;
    int last_use_time;
    unsigned used :1 ;
    unsigned frameIndex : 7;
    unsigned page_Index : 6;
} frame_t;

typedef struct {
    unsigned long long age :32;
} age_t;

struct INSTR {
	string type;
	int page;
} instruction;

vector<INSTR> instrList;

// Global frame table

int num_frames;
pte_t page_table[MAX_VPAGE];


int myrandom(int burst);


class VMA{
    public:
        int start_vpage;
        int end_vpage;
        bool write_protected;
        bool file_mapped;
        VMA(){};

        VMA(int start_vpage,int end_vpage,int write_protected,int file_mapped){
            this->start_vpage = start_vpage;
            this->end_vpage = end_vpage;
            this->write_protected = write_protected;
            this->file_mapped = file_mapped;
        };
};

class PageTable{
    public:
        int pid;
        list <VMA> VMA_LIST;
        vector <pte_t> page_table;

        int map_counter=0;
        int unmap_counter=0;
        int in_counter=0;
        int out_counter=0;
        int fin_counter=0;
        int fout_counter=0;
        int zero_counter=0;
        int segv_counter=0;
        int segprot_counter=0;

        PageTable(){};

        PageTable(int pid,list <VMA> VMA_LIST, vector <pte_t> page_table){
            this->pid = pid;
            this->VMA_LIST = VMA_LIST;
            this->page_table = page_table;
        };



};

map<int, PageTable> page_table_set;

class Pager{
    public:
        map<int,frame_t> frame_table;
    
        virtual frame_t* select_victim_frame() = 0;
        virtual frame_t* select_victim_frame_free(int index) = 0;
        void print_frame_table(){
            //print the frame table status
            cout << "FT: ";
            for(int i=0; i<num_frames; i++){
                frame_t *temp_frame = &frame_table[i];
                if(temp_frame->used){
                    cout << temp_frame->pid << ":" << temp_frame->page_Index << " ";
                }else{
                    cout << "* ";
                }

            }
            cout << endl;
        }
        
};

class F_Pager: public Pager{
    public:
        map<int,frame_t> frame_table;
        int last_frame = num_frames-1;
        frame_t* select_victim_frame(){
            //int temp_time = 0;
            frame_t* temp_frame;
            if(last_frame == num_frames-1){
                last_frame = 0;
            }else{
                last_frame++;
            }     
            temp_frame = &frame_table[last_frame];
            temp_frame->frameIndex = last_frame;
            return temp_frame;
        };
        frame_t* select_victim_frame_free(int index){
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
};

class R_Pager: public Pager{
    public:
        map<int,frame_t> frame_table;
        int last_frame = -1;
        frame_t* select_victim_frame(){
            frame_t* temp_frame;
            if(last_frame == num_frames-1){
                int rand_frame_idx = myrandom(num_frames);
                temp_frame = &frame_table[rand_frame_idx];
                //temp_frame->frameIndex = rand_frame_idx;
                return temp_frame;
            }else{
                last_frame++;
            }     
            temp_frame = &frame_table[last_frame];
            temp_frame->frameIndex = last_frame;
            return temp_frame;

            
        };
        frame_t* select_victim_frame_free(int index){
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
};

class C_Pager: public Pager{
    public:
        map<int,frame_t> frame_table;
        int hand = 0;
        frame_t* select_victim_frame(){
            frame_t* temp_frame;
            while(true){
                if(hand == num_frames){
                    hand = 0;
                }
                if(frame_table[hand].used){         
                    pte_t *old_pte = &page_table_set[frame_table[hand].pid].page_table[frame_table[hand].page_Index];
                    int frame_pass = 0;
                    //cout << "ASELECT: " << hand;
                    if(old_pte->referenced){
                        old_pte->referenced = {0};
                        hand++;
                        frame_pass++;
                        continue;
                    }else{
                        //cout << " " << frame_pass << endl;
                        temp_frame = &frame_table[hand];
                        temp_frame->frameIndex = hand;
                        hand++;
                        break;
                    }
                }else{
                    temp_frame = &frame_table[hand];
                    temp_frame->frameIndex = hand;
                    hand++;
                    break;
                }
            }
            
            return temp_frame;
        };
        frame_t* select_victim_frame_free(int index){
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
};

int cpu_clock = 0;

class E_Pager: public Pager{
    public:
        map<int,frame_t> frame_table;
        int hand = 0;
        int temp_hand = 0;
        int temp_class = 0;
        int chosen_class = 4;
        bool ref_rest = false;
        

        frame_t* select_victim_frame(){
            frame_t* temp_frame;
            for(int i=0;i<num_frames;i++){
                if(!frame_table[hand].used){
                    //cout << "choose a new frame" << "with  index:" << hand << " " << frame_table[hand].frameIndex << endl;
                    temp_hand = hand;
                    temp_frame = &frame_table[temp_hand];
                    temp_frame->frameIndex = temp_hand;
                    hand++;
                    hand %= num_frames;
                    break;
                }
                if(cpu_clock>=50){
                    ref_rest = true;
                    cpu_clock = 0;
                }
                //cout << "choose an old frame" << "with  index:" << hand << " " << frame_table[hand].frameIndex << endl;
                pte_t *old_pte;
                old_pte = &page_table_set[frame_table[hand].pid].page_table[frame_table[hand].page_Index];
                //cout << "the pte inside has " << old_pte->referenced << " " << old_pte->modified << endl;
                temp_class = old_pte->referenced*2+old_pte->modified;
                //cout << "the class of this pte:" << temp_class<< endl;
                //cout << "the chosen class right now is: " << chosen_class << endl;
                if(ref_rest){
                    old_pte->referenced = {0};
                }
                if(temp_class == 0 && !ref_rest){
                    temp_hand = hand;
                    temp_frame = &frame_table[temp_hand];
                    temp_frame->frameIndex = temp_hand;
                    break;
                }
                if(temp_class < chosen_class){
                    //cout << "update the temp chosen" << endl;
                    temp_hand = hand;
                    temp_frame = &frame_table[temp_hand];
                    temp_frame->frameIndex = temp_hand;
                    chosen_class = temp_class;
                }
                hand++;
                hand %= num_frames;
            }

            ref_rest = false;
            hand = temp_hand+1;
            hand %= num_frames;
            chosen_class = 4;
            
            return temp_frame;
        };
        frame_t* select_victim_frame_free(int index){
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
};

class A_Pager: public Pager{
    public:
        map<int,frame_t> frame_table;
        int hand = 0;
        vector<age_t> age_bit_vec;
        
        frame_t* select_victim_frame(){
            int temp_hand = hand;
            frame_t* temp_frame;
            for(int i = 0; i < num_frames; i++){
                //cout << "in" <<  endl;
                temp_frame = &frame_table[hand];
                if(!temp_frame->used){
                    temp_hand = hand;
                    hand++;
                    hand %= num_frames;
                    age_bit_vec.push_back({0});
                    temp_frame->frameIndex = temp_hand;
                    //cout << temp_frame->frameIndex << endl;
                    return temp_frame;
                }
            }
            //cout << "in" <<  endl;
            
            //update
            for(int i=0; i<num_frames; i++){
                age_bit_vec[i].age = age_bit_vec[i].age >> 1;
                pte_t *old_pte = &page_table_set[frame_table[i].pid].page_table[frame_table[i].page_Index];
                if(old_pte->referenced){
                    age_bit_vec[i].age |= 0x80000000;
                }
            }

            //select
            unsigned long long temp_age = age_bit_vec[temp_hand].age;
            hand = temp_hand;
            for(int i=0; i < num_frames; i++){
                if(age_bit_vec[hand].age < temp_age){
                    temp_hand = hand;
                    temp_age = age_bit_vec[hand].age;
                }
                hand++;
                hand %= num_frames;
            }
            hand = temp_hand+1;
            hand %= num_frames;
            age_bit_vec[temp_hand].age = 0;
            for(int i=0; i<num_frames; i++){
                if(frame_table[i].used){
                    pte_t *rst_pte;
                    rst_pte = &page_table_set[frame_table[i].pid].page_table[frame_table[i].page_Index];
                    rst_pte->referenced = {0}; 
                }
            }
            temp_frame = &frame_table[temp_hand];
            return temp_frame;
        };
        frame_t* select_victim_frame_free(int index){
            age_bit_vec[index].age = 0;
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
};

class W_Pager: public Pager{
    public:
        // still consider cpu_counter as the time counter

        map<int,frame_t> frame_table;
        int hand = 0;
        
        frame_t* select_victim_frame(){
            int temp_hand = 0;
            frame_t* temp_frame;
            int temp_dur = 0;
            for(int i=0; i < num_frames; i++)
            {
                if(!frame_table[hand].used){
                    temp_hand = hand;
                    hand++;
                    hand %= num_frames;
                    temp_frame = &frame_table[temp_hand];
                    temp_frame->frameIndex = temp_hand;
                    break;
                }
                pte_t *old_pte;
                old_pte = &page_table_set[frame_table[hand].pid].page_table[frame_table[hand].page_Index];
                temp_frame = &frame_table[hand];
                if(old_pte->referenced){
                    temp_frame->last_use_time = cpu_clock;
                    old_pte->referenced = {0};
                }else{
                    if(cpu_clock-temp_frame->last_use_time > 49){
                        //temp_frame->frameIndex = hand;
                        temp_hand = hand;
                        //hand++;
                        //hand %= num_frames;
                        break;
                    }else{
                        if(cpu_clock-temp_frame->last_use_time > temp_dur){
                            //cout << "update the temp frame from " << temp_hand  << " to " << hand <<endl;
                            temp_dur = cpu_clock-temp_frame->last_use_time;
                            //temp_frame->frameIndex = hand;
                            temp_hand = hand;
                            // hand++;
                            // hand %= num_frames;                            
                        }

                    }
                } 
                hand++;
                hand %= num_frames;
                
            }
            hand = temp_hand+1;
            hand %= num_frames;
            temp_frame = &frame_table[temp_hand];

            return temp_frame;
  

            // cout << hand << endl;
            // frame_t* temp_frame;
            // int temp_dur = 0;
            // int temp_hand = hand;
            // int end_point = 0;
            // for(int i = 0; i < num_frames; i++){
            //     if(frame_table[hand].used){         
            //         pte_t *old_pte = &page_table_set[frame_table[hand].pid].page_table[frame_table[hand].page_Index];
            //         cout << " The pte for this hand:" << frame_table[hand].pid << ":" << frame_table[hand].page_Index <<
            //         " with reference" << old_pte->referenced << " and last time:" << frame_table[hand].last_use_time << endl;
            //         //int frame_pass = 0;
            //         //cout << "ASELECT: " << hand;
            //         if(old_pte->referenced){
            //             temp_frame = &frame_table[hand];
            //             temp_frame->last_use_time = cpu_clock;
            //             old_pte->referenced = {0};
            //             //frame_pass++;
            //             continue;
            //         }else{
            //             //cout << " " << frame_pass << endl;
            //             temp_frame = &frame_table[hand];
            //             if(cpu_clock - temp_frame->last_use_time > 49){
            //                 temp_frame->frameIndex = hand;
            //                 break;
            //             }else{
            //                 if(cpu_clock - temp_frame->last_use_time > temp_dur){
            //                     //temp_frame->frameIndex = hand;
            //                     temp_dur = cpu_clock - temp_frame->last_use_time;
            //                     temp_hand = hand;
            //                     cout << "update with:" << temp_hand << " new time_dur:" << temp_dur << endl;
            //                 }
            //                 temp_frame->frameIndex = hand;
            //             }
            //         }
            //     }else{
            //         temp_frame = &frame_table[hand];
            //         //temp_frame->used = {1};
            //         temp_frame->frameIndex = hand;
            //         temp_hand = hand;
            //         break;
            //     }

            //     hand++;
            //     if(hand == num_frames){
            //         hand = 0;
            //     }
            //     //cout << "round hand" << hand << endl;
            // }
            // temp_frame = &frame_table[temp_hand];

            // //cout << "final hand" << temp_hand << endl;
            
        };
        frame_t* select_victim_frame_free(int index){
            frame_t* temp_frame = &frame_table[index];
            temp_frame->frameIndex = index;
            temp_frame->used = {0};
            return temp_frame;
        };
        
};





//random var
vector<int> random_num_array;
int size_of_random_array = 0;
int ofs = 0;

Pager *THE_Pager;

list <INSTR> INSTR_LIST;

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
    infile.close();
}

int myrandom(int burst){
    int temp_return_val = (random_num_array[ofs] % burst);
    ofs ++;
    if(ofs == size_of_random_array){
        ofs = 0;
    }
    return temp_return_val;
}

//vector<PageTable> page_table_list;
// initilize the input file


void load_input_file(char* inputfileName){
    ifstream infile(inputfileName);
    string line;
    int pro_num = 0;
    int vma_num = 0;
    if(infile.is_open()){
        while (getline(infile, line)){
            if(line.find("#") == string::npos){
                pro_num = atoi(line.c_str());
                break;
            }
        }
        int pid = -1;
        while(pro_num > 0){
            getline(infile, line);
            if(line.find("#") != string::npos)
                continue;
            pro_num --;
            pid ++;
            PageTable pt = PageTable();
            pt.pid = pid;
            vma_num = atoi(line.c_str());
            //cout << "vmaNum: " << vmaNum <<endl;
            while(vma_num > 0){
                getline(infile, line);
                if(line.find("#") != string::npos)
                    continue;
                vma_num --;
                istringstream is(line);
                VMA vma = VMA();
                is >> vma.start_vpage >> vma.end_vpage >> vma.write_protected >> vma.file_mapped;
                pt.VMA_LIST.push_back(vma);
                //cout << "vma: " << object_vma.end_page << endl;     
                vector<pte_t> temp_tab (MAX_VPAGE);    
                pt.page_table = temp_tab;
            }
            page_table_set[pt.pid] = pt;
            //cout << "size: "<< object.vmaList.size()<<endl;
        }

        while (getline(infile, line)){
            if(line.find("#") == string::npos){
                istringstream is(line);
                INSTR instr = INSTR();
                is >> instr.type >> instr.page;
                INSTR_LIST.push_back(instr);
            }

        }
    }
    infile.close();
}
list<frame_t*> free_pool;

frame_t *alllocate_frame_from_free_list(){
    if(!free_pool.empty()){
        frame_t *temp_frame = free_pool.front();
        free_pool.pop_front();
        return THE_Pager->select_victim_frame_free(temp_frame->frameIndex);
    }
    else{
        return nullptr;  
    }
    return nullptr;
}


frame_t *get_frame(){
    frame_t *frame = alllocate_frame_from_free_list();
    if(frame == nullptr){ 
        frame = THE_Pager->select_victim_frame();      
    }
    return frame;
}


bool get_next_instruction(string &operation, int &vpage){
    if(INSTR_LIST.size() > 0){  
        INSTR temp_instr = INSTR_LIST.front();
        INSTR_LIST.pop_front(); 
        operation = temp_instr.type;
        vpage = temp_instr.page;
        cpu_clock ++;
              
        return true;
    }else{
        return false;
    }
}

int inst_count; 
int ctx_switches; 
int process_exits;
int cost;
bool segv_trigger = false;
bool segprot_trigger = false;

bool O_trigger = false;
bool P_trigger = false;
bool F_trigger = false;
bool S_trigger = false;

void Simulation(){
    int current_process;
    string operation;
    int vpage;
    int current_instr = 0;
    
    while(get_next_instruction(operation, vpage)){
        inst_count++;
        if(operation == "c"){
            if(O_trigger)
                cout << current_instr << ": ==> " << operation << " " << vpage << endl;
            current_instr++;
            current_process = vpage;

            ctx_switches++;
            cost +=130;
            continue;
        }else if(operation == "e"){
            if(O_trigger)
                cout << current_instr << ": ==> " << operation << " " << vpage << endl;
            current_instr++;
            if(O_trigger)
                cout << "EXIT current process " << current_process << endl;
            
            for(int i=0; i <MAX_VPAGE; i++){
                pte_t *temp_pte = &page_table_set[current_process].page_table[i];
                if(temp_pte->present){
                    if(O_trigger)
                        cout << " UNMAP " << current_process << ":" << i << endl;
                    cost+=400;
                    page_table_set[current_process].unmap_counter++;
                    if(temp_pte->modified && temp_pte->filemapped){
                        page_table_set[current_process].fout_counter += 1;
                        cost += 2400;
                        if(O_trigger)
                            cout <<" FOUT" << endl;
                    }
                    frame_t *frame = &THE_Pager->frame_table[temp_pte->frameIndex];
                    free_pool.push_back(frame);
                    frame->used = {0};
                    frame->page_Index = {0};

                }
                temp_pte->modified = {0};
                temp_pte->frameIndex = {0};
                temp_pte->present = {0};
                temp_pte->pagedout = {0};
            }
            current_process = 0;
            process_exits++;
            cost+=1250;
            continue;
        }

        //now only 'w' or 'r'
        cost++;
        //cout << "current page" << current_process << ":" << vpage <<endl;
        
        pte_t *pte = &page_table_set[current_process].page_table[vpage];
        if(!pte->referenced){
            list<VMA>::iterator iter;
            for(iter = page_table_set[current_process].VMA_LIST.begin(); iter != page_table_set[current_process].VMA_LIST.end() ;++iter){
                if(iter->start_vpage <= vpage && vpage <= iter->end_vpage){
                    pte->write_protect = iter->write_protected;
                    pte->filemapped = iter->file_mapped;
                    pte->non_hole = {1};
                }
            }
        }
        if(pte->non_hole){
            if(operation == "w"){
                if(pte->write_protect){
                    pte->referenced = {1};
                    page_table_set[current_process].segprot_counter++;
                    cost +=420;
                    segprot_trigger = true;
                }else{
                    pte->modified = {1};
                    pte->referenced = {1};
                }

            }else if(operation == "r"){
                pte->referenced = {1};
            }
            if(O_trigger)
                cout << current_instr << ": ==> " << operation << " " << vpage << endl;
            current_instr++;
            //cout << " idx: " << pte->frameIndex << " pre" << pte->present << "ref" << pte->referenced <<  endl;
            if(!pte->present){
                frame_t *newframe = get_frame();
                if(newframe->used){
                    pte_t *old_pte = &page_table_set[newframe->pid].page_table[newframe->page_Index];
                    old_pte->present = {0};  
                    if(O_trigger)         
                        cout << " UNMAP " << newframe->pid << ":" << newframe->page_Index << endl;
                    cost+=400;
                    page_table_set[newframe->pid].unmap_counter++;
                    if(old_pte->modified){
                        if(!old_pte->filemapped)
                            old_pte->pagedout = {1};
                        if(old_pte->filemapped){
                            if(O_trigger)
                                cout << " FOUT" << endl;
                            cost+=2400;
                            page_table_set[newframe->pid].fout_counter++;
                        }else{
                            if(O_trigger)
                                cout << " OUT" << endl;
                            cost+=2700;
                            page_table_set[newframe->pid].out_counter++;
                        }
                        
                    }
                    if(old_pte->modified){
                        if(!old_pte->filemapped)
                            old_pte->pagedout = {1};
                    }
                    old_pte->modified = {0};

                }
                if(pte->filemapped){
                    if(O_trigger)
                        cout << " FIN" << endl;
                    cost+=2800;
                    page_table_set[current_process].fin_counter++;
                }
                else if(pte->pagedout){
                    if(pte->filemapped){
                        if(O_trigger)
                            cout << " FIN" << endl;
                        cost+=2800;
                        page_table_set[current_process].fin_counter++;
                    }else{
                        if(O_trigger)
                            cout << " IN" << endl;
                        cost+=3100;
                        page_table_set[current_process].in_counter++;
                    }
                }else{
                    if(O_trigger)
                        cout << " ZERO" << endl;
                    cost+=140;
                    page_table_set[current_process].zero_counter++;
                }
                if(O_trigger)
                    cout << " MAP " << newframe->frameIndex << endl;
                
                cost+=300;
                page_table_set[current_process].map_counter++;
                newframe->used = {1};
                newframe->pid = current_process;
                newframe->page_Index = vpage;
                THE_Pager->frame_table[newframe->frameIndex].used = {1};
                THE_Pager->frame_table[newframe->frameIndex].pid = current_process;
                THE_Pager->frame_table[newframe->frameIndex].page_Index = vpage;
                THE_Pager->frame_table[newframe->frameIndex].frameIndex = newframe->frameIndex;
                pte->frameIndex = newframe->frameIndex;
                pte->present = {1};
                pte->referenced = {1};
            }
        }else{
            if(O_trigger)
                cout << current_instr << ": ==> " << operation << " " << vpage << endl;
            current_instr++;
            if(O_trigger)
                cout << " SEGV" << endl;;
            page_table_set[current_process].segv_counter++;
            cost +=340;
        }
        if(segprot_trigger){
            if(O_trigger)
                cout << " SEGPROT" << endl;
            segprot_trigger = false;
        }
        
        //cout << " idx: " << pte->frameIndex << " pre" << pte->present << "ref" << pte->referenced <<  endl;
    }

    //print these page_table status
    if(P_trigger){
        for(int i=0; i<page_table_set.size(); i++){
            cout << "PT[" << i << "]: ";
            PageTable temp_pt = page_table_set[i];
            for(int j = 0; j <MAX_VPAGE; j++){
                pte_t temp_pte = temp_pt.page_table[j];
                if(temp_pte.present){
                    cout << j << ":";
                    string temp_str;
                    if(temp_pte.referenced){                
                        temp_str.append("R");
                    }else{
                        temp_str.append("-");
                    }
                    if(temp_pte.modified){
                        temp_str.append("M");
                    }else{
                        temp_str.append("-");
                    }
                    if(temp_pte.pagedout){
                        temp_str.append("S");
                    }else{
                        temp_str.append("-");
                    }
                    cout << temp_str << " ";
                }else{
                    if(temp_pte.pagedout){
                        cout << "# ";
                    }else{
                        cout << "* ";
                    }
                }
            
            }
            cout << endl;
        }
    }

    if(F_trigger){
        THE_Pager->print_frame_table();
    }
    
    if(S_trigger){
    //print the counter information for each process
        for(int i=0; i<page_table_set.size(); i++){
            cout << "PROC[" << i << "]:";
            PageTable temp_pt = page_table_set[i];
            cout << " U=" << temp_pt.unmap_counter
                << " M=" << temp_pt.map_counter
                << " I=" << temp_pt.in_counter
                << " O=" << temp_pt.out_counter
                << " FI=" << temp_pt.fin_counter
                << " FO=" << temp_pt.fout_counter
                << " Z=" << temp_pt.zero_counter
                << " SV=" << temp_pt.segv_counter
                << " SP=" << temp_pt.segprot_counter << endl;;
        }

    //print the totalcost

        cout << "TOTALCOST " << inst_count << " " 
                            << ctx_switches << " " 
                            << process_exits << " "
                            << cost << " "
                            << sizeof(pte_t) <<  endl;
    }
}


int main(int argc, char* argv[]){

    frame_t frame_table[1];
    char* inputFileName;
    char* randomFileName;
    int c;
    
    int index;
    string vmm_algo;
    string output;
    while((c = getopt (argc, argv, "f:a:o:")) != -1)
        switch(c)
            {
            case 'f':
                num_frames = atoi(optarg);
                break;
            case 'a':
                vmm_algo = optarg;
                //cout << "get a" << endl;
                break;
            case 'o':
                output = optarg;
                //cout << "get o" << endl;
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
    if(vmm_algo.at(0) == 'f'){
        THE_Pager = new F_Pager();
        //cout << "get f pager" << endl;
    }
    else if(vmm_algo.at(0) == 'r'){
        THE_Pager = new R_Pager();
    }
    else if(vmm_algo.at(0) == 'c'){
        THE_Pager = new C_Pager();
    }
    else if(vmm_algo.at(0) == 'e'){
        THE_Pager = new E_Pager();
    }
    else if(vmm_algo.at(0) == 'a'){
        THE_Pager = new A_Pager();
    }
    else if(vmm_algo.at(0) == 'w'){
        THE_Pager = new W_Pager();
    }

    if(output.find("O") != string::npos){
        O_trigger = true;
    }
    if(output.find("P") != string::npos){
        P_trigger = true;
    }
    if(output.find("F") != string::npos){
        F_trigger = true;
    }
    if(output.find("S") != string::npos){
        S_trigger = true;
    }


    load_input_file(inputFileName);
    loading_random_num_file(randomFileName);
    
    Simulation();
}



