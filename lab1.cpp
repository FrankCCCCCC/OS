#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>
#include <iostream>
#include <iomanip> 
#include <string> 
#include <cstring>
#include <vector> 
#include <algorithm>

using namespace std;

//Copyright
//Operating Systems CSCI-GA.2250-001, Spring 2021
//ID:zc2244

//structures declaration
struct definition_list{
    int def_count;
    vector<string> def_sym_list;
    vector<int> def_addr_list;
};

struct use_list{
    int use_count;   
    vector<string> defSymList;   
};

struct program_text{
    int code_count;
    vector<char> type_list;
    vector<int> instr_list;
};

struct module{
    int address;
    struct definition_list definition_list;
    struct use_list use_list;
    struct program_text program_text;
};

struct Symbol{
    string sym;
    int val; 
};



//functions pre-declaration
char* get_token(ifstream &infile, char* buffer);
void read_line(ifstream &infile, char *buffer);
int readInt(ifstream &infile, char* buffer);
char* readSymbol(ifstream &infile, char* buffer);
char readIAER(ifstream &infile, char* buffer);
void Pass1(ifstream &infile);
void Pass2(ifstream &infile);
void symbol_table_printer();
void __parseerror(int errcode);
void createModule();


//Global variables declaration

char* fileName;
char delim[]=" \t \r \v \n \f";
bool trigger1 = true;
bool trigger2 = true;
int line_num = 0;
int line_offset = 0;
//char *buffer;
//char buffer_[256];
string line_token;
vector <Symbol> sym_table;
vector <string> sym_error;
vector <string> warning_table;
vector <module> module_table;
int temp_offset = 0;


//Main function
int main(int argc, char* argv[]){
    if (argc != 2 ){
        cout << "Error: file input format wrong" << endl;
        exit(0);
    }
    fileName = argv[1];
    ifstream infile(fileName);

    ///getToken(argv[1]);
    Pass1(infile);
    infile.close();
    symbol_table_printer();
    infile.open(fileName);
    Pass2(infile);


}

//Macros used in checking functions

//isdigit() has already been bulit, same as isalpha() and isalnum()

bool isIAER(char token){
    return (token == 'I') || (token == 'A') || (token=='E') || (token=='R');
}

//Checking functions
int readInt(ifstream &infile, char* buffer){

    string token = get_token(infile, buffer);

    for(int i=0; i<token.length(); i++){
        if(!isdigit(token[i])){  
            __parseerror(0);       
            exit(0);
        }
    }

    return atoi(token.c_str());
}

char* readSymbol(ifstream &infile, char* buffer){
    char* token;
    token = get_token(infile,buffer);
    string temp = token;
    if(!isalpha(temp[0])){
        __parseerror(1);
    }
    for(int i=1; i<temp.length(); i++){
        
        if(!isalnum(temp[i])){  
            __parseerror(1);
            exit(0);
        }
    }
    if(temp.length() > 16){
            __parseerror(3);
            exit(0);
    }
    return token;
}

char readIAER(ifstream &infile, char* buffer){
    string temp = get_token(infile, buffer);

    if(temp.length() != 1){
        __parseerror(2);
        exit(0);
    }
    
    char temp_char = temp[0];
    if(!isIAER(temp_char)){
        __parseerror(2);
        exit(0);
    }
    return temp_char;
}

//Parse Error function
void __parseerror(int errcode) {
    static string errstr[] = {
    "NUM_EXPECTED", // Number expect
    "SYM_EXPECTED", // Symbol Expected
    "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R
    "SYM_TOLONG", // Symbol Name is too long
    "TO_MANY_DEF_IN_MODULE", // > 16
    "TO_MANY_USE_IN_MODULE", // > 16
    "TO_MANY_INSTR", // total num_instr exceeds memory size (512)
    };
    cout << "Parse Error line " << line_num << " offset " << line_offset << " : " << errstr[errcode] <<endl;
    //printf("Parse Error line %d offset %d: %s\n", line_num, line_offset, errstr[errcode]);
}

void read_line(ifstream &infile, char *buffer){
    infile.getline(buffer, 100);
    line_token = buffer;
    line_num ++;
    line_offset = 0;
    trigger2 = false;
    trigger1 = true;
}

//Function:get_token (tokenizer)
char* get_token(ifstream &infile, char* buffer){
    char* token;
    
    // if(trigger){    
    //     token = strtok(buffer,delim);
    //     trigger = false;
    // }
    // else{
    //     //cout <<"trigger" <<endl;
    //     token = strtok(NULL, delim);
    // }
    // if (token == NULL){
    //     //cout << buffer_ << endl;
    //     infile.getline(buffer,100);
    //     //token = get_token(infile);
    //     if(strlen(buffer_) != 0){
    //         token = strtok(buffer_, delim);
    //     }else{
    //         cout << "getline" <<endl;
    //         infile.getline(buffer_,100);
    //     }       
    // }
    // return token;
    if(trigger2){
        read_line(infile, buffer);
    }
    if(trigger1){
        token = strtok(buffer, delim);
        trigger1 = false;
    }else{
        token = strtok(NULL, delim);
    }
    if(token == NULL){
        read_line(infile, buffer);
        token = get_token(infile, buffer);

    }  
    line_offset = int(line_token.find(token, line_offset)) + 1;
    return token;
}

//The find fcuntion for checking the repetitive definition
bool find(Symbol new_sym){
    bool tmp_boo = false;
	for(auto temp : sym_table){
		if( temp.sym == new_sym.sym )
			tmp_boo = true;
	}
	return tmp_boo;
}

int find_sym(string sym){
	for(auto temp : sym_table){
		if( temp.sym == sym )
			return temp.val;
	}
	return 0;
}

//Create function
void creatSymbol(string sym, int val){
    int temp_val = val + temp_offset;
    Symbol temp = {sym,temp_val};
    if (find(temp)){
        sym_error.push_back(temp.sym);
    }
    else{
        sym_table.push_back(temp);
    }
}

void createModule(){
    module new_module = module();
    module_table.push_back(new_module);
}

void map_printer(char addressmode, int operand, int map_count, int module_offset, int module_num){
    if(addressmode == 'I'){
        cout << setw(3) << setfill('0') << map_count << ": " << operand << endl;
    }
    else if(addressmode == 'A'){
        cout << setw(3) << setfill('0') << map_count << ": " << operand << endl;
    }
    else if(addressmode == 'E'){
        int temp_op = operand;
        temp_op += find_sym(module_table[module_num].use_list.defSymList[0]); 
        cout << setw(3) << setfill('0') << map_count << ": " << temp_op << endl;
    }
    else if(addressmode == 'R'){
        int temp_op = operand;
        temp_op += module_offset; 
        cout << setw(3) << setfill('0') << map_count << ": " << temp_op << endl;
    }
}
//Pass1: first parser
void Pass1(ifstream &infile){
    char* token;
    char buffer[256];
    
    if (! infile.is_open()) { 
        cout << "Error opening file"; 
        exit (0); 
    }
    
    while (infile.peek() != EOF) {     
        //cout << get_token(infile) << endl;
        int defcount =  readInt(infile, buffer);
        cout << defcount << "\t";
        for(int i=0; i<defcount; i++){
            string sym = readSymbol(infile, buffer);
            cout << sym << "\t";
            int val = readInt (infile, buffer);
            cout << val << "\t";
            creatSymbol(sym,val);
        }
        cout << endl;
        int usecount = readInt(infile, buffer);
        cout << usecount << "\t";
        for(int i=0; i<usecount; i++){
            string sym = readSymbol(infile, buffer);
            cout << sym << "\t";
        }
        cout << endl;
        int instcount = readInt(infile, buffer);
        cout <<instcount << "\t";
        for(int i=0; i<instcount; i++){
            char addressmode = readIAER(infile, buffer);
            cout << addressmode << "\t";
            int operand = readInt(infile, buffer);
            cout << operand << "\t";
        }
        cout << endl;
        temp_offset += instcount;
    } 
}

void symbol_table_printer(){
    cout << "Symbol Table" << endl;
    // for(auto temp1: sym_table, sym_error){
    //     cout << temp1.sym << '=' << temp1.val;
    //     if(temp2.error){
    //         cout << "  Error: This variable is numtiple times defined; first value used";
    //     }
    //     cout << endl;
    // }
    for (int i=0; i < sym_table.size(); i++){
        cout << sym_table[i].sym << '=' << sym_table[i].val;
        for(int j=0; j < sym_error.size(); j++){
            if(sym_error[j] == sym_table[i].sym){
                cout << "  Error: This variable is numtiple times defined; first value used";
            }
        }      
        cout << endl;
    }
    
}

//Pass2: second parser
void Pass2(ifstream &infile){
    char buffer[256];
    int map_count = 0;
    int module_num = 0;
    line_num = 0;
    if (! infile.is_open()) { 
        cout << "Error opening file"; 
        exit (0); 
    }
    
    while (infile.peek() != EOF) {       
        createModule();
        //module temp_module = module_table[module_num];
        //cout << get_token(infile) << endl;
        int module_offset = map_count;
        int defcount =  readInt(infile, buffer);
        module_table[module_num].definition_list.def_count = defcount;
        //cout << defcount << "\t";
        for(int i=0; i<defcount; i++){
            string sym = readSymbol(infile, buffer);
            module_table[module_num].definition_list.def_sym_list.push_back(sym);
            //cout << sym << "\t";
            int val = readInt (infile, buffer);
            module_table[module_num].definition_list.def_addr_list.push_back(val);
            //cout << val << "\t";
        }
        //cout << endl;
        int usecount = readInt(infile, buffer);
        module_table[module_num].use_list.use_count = usecount;
        //cout << usecount << "\t";
        for(int i=0; i<usecount; i++){
            string sym = readSymbol(infile, buffer);
            module_table[module_num].use_list.defSymList.push_back(sym);
            //cout << sym << "\t";
        }
        //cout << endl;
        int instcount = readInt(infile, buffer);
        module_table[module_num].program_text.code_count = instcount;
        //cout <<instcount << "\t";
        for(int i=0; i<instcount; i++){
            char addressmode = readIAER(infile, buffer);
            module_table[module_num].program_text.type_list.push_back(addressmode);
            //cout << addressmode << "\t";
            int operand = readInt(infile, buffer);
            module_table[module_num].program_text.instr_list.push_back(operand);
            //cout << operand << "\t";
            map_printer(addressmode, operand, map_count, module_offset, module_num);
            map_count ++;
        }
        module_num ++;
    } 
    infile.close();
}