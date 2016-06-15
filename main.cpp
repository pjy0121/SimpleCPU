//Danielle Rubio
//CS 4348
//Project 1
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>


using namespace std;
int data[2000];

int read(int address);
void write(int address, string value);
void mem(string filename);
void checkPC(int PC, bool mode);


int main(int argc, char *argv[]){
    int cpu_mem[2];
    int mem_cpu[2];
    if(argc != 3){
        cout << "Invalid Format" << endl;
        cout << "Please enter the following: ./main program.txt interruptTime" << endl;
    }
    string filename = string(argv[1]);
    int time = atoi(argv[2]);
    int result;
    pipe(cpu_mem);
    pipe(mem_cpu);
    result = fork();
    
    if(result == -1){
        exit(1);
    }
    if(result == 0){
		close(cpu_mem[1]);
		close(mem_cpu[0]);
        //child process memory
        //instantiate the memory
        
        mem(filename);
        //cout << "memory instantiated" << endl;
        int index;
        int instr;
        int val;
        //flag variable to decide if exit, read, write
        int flag; // -1 = exit, 0 = read, 1 = write
        while(true){
			
            read(cpu_mem[0], &flag, sizeof(flag));
            if(flag == 0){ //read from mem
                read(cpu_mem[0], &index, sizeof(index)); //index holds PC
				
                instr = read(index);
                write(mem_cpu[1], &instr, sizeof(instr)); //write instruction to CPU
            }
            else if(flag == 1){ //write to mem
                read(cpu_mem[0], &index, sizeof(index)); //find index
                read(cpu_mem[0], &val, sizeof(val)); //val holds item to be store in mem
                data[index] = val; //write to memory
            }
            else{
                exit(1);
            }
        }
        
    }
    else{
		close(cpu_mem[0]);
		close(mem_cpu[1]);
        //Interrupts Occur At:
        //timer, after every T instructions
        //system call using 29
        bool CPU = true;
        int PC = 0; //program count
        int SP = 1000; //value of stack pointer
        int AC = 0; //ac
        int IR = 0; //instruction register
        int X = 0; //value of X
        int Y = 0; // value of Y
        bool kernel_mode = false; //initially in user mode
        //check if in sys_mode, and trying to access PC 1000-1999, output error mode
        //only goes in sys_mode during interrupt
        int flag; //flag to tell mem read or write
        int value; //variable to hold information
        int IC = 0; //instruction count for timer
        //push forward, pop backwards,
        //SP will become system stack, save the PC and SP, and push onto the system stack, then pop them off in reverse
        while(CPU){
            //cout << "FETCHING" << endl;
			
           if((IC!=0) && (kernel_mode == false)&& ((IC%time)== 0)){
               kernel_mode = true;
               int stack;
               int prog_count;
               stack = SP;
               prog_count = PC;
               SP = 2000;
               //tell memory gonna write
               flag = 1;
               write(cpu_mem[1], &flag, sizeof(flag));
               SP--;
               write(cpu_mem[1],&SP, sizeof(SP));
               write(cpu_mem[1], &stack, sizeof(stack));//push stack
               
               write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
               SP--;
               write(cpu_mem[1], &SP, sizeof(SP));
               write(cpu_mem[1], &prog_count, sizeof(prog_count)); //push PC
			   
			   write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
               SP--;
               write(cpu_mem[1], &SP, sizeof(SP));
               write(cpu_mem[1], &AC, sizeof(AC)); //push AC
			   
			   write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
               SP--;
               write(cpu_mem[1], &SP, sizeof(SP));
               write(cpu_mem[1], &IR, sizeof(IR)); //push IR
			   
			   write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
               SP--;
               write(cpu_mem[1], &SP, sizeof(SP));
               write(cpu_mem[1], &X, sizeof(X)); //push X
			   
			   write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
               SP--;
               write(cpu_mem[1], &SP, sizeof(SP));
               write(cpu_mem[1], &Y, sizeof(Y)); //push Y
			   
               PC = 1000;
               IC = 0;
               continue;
           }
           if(kernel_mode == false){
               //only count in user mode
               IC++;
           }
            
            flag = 0; // need to read from memory initially
            //write PC to memory
            write(cpu_mem[1], &flag, sizeof(flag)); //only reading for the IR
            write(cpu_mem[1], &PC, sizeof(PC)); //write the address
            // CPU read from mem
            read(mem_cpu[0], &IR, sizeof(IR)); //read the instruction
			
			if (IR == 29 && kernel_mode)
			{
				PC++;
				continue;
			}
            PC++; // increment PC
            switch(IR){
                case 1: //LoadValue
                    //cout <<"C1" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    PC++;
                    break;
                case 2: //Load Address
                    //cout <<"C2" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    checkPC(value, kernel_mode);//check if address is valid before accessing
                    flag = 0; //read from mem
                    write(cpu_mem[1], &flag, sizeof(flag));
                    //get the value at the address
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    PC++;
                    break;
                case 3: //LoadIndAddr holds 500, contains 100, load from 100
                    //cout <<"C3" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    //value holds address, value = 500
                    checkPC(value, kernel_mode);//check if address is valid before accessing
                    flag = 0; //read from mem
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    
                    flag = 0; //read from mem
                    write(cpu_mem[1], &flag, sizeof(flag));
                    //value == 100, load from 100
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    PC++;
                    break;
                case 4://LoadIdxX value at (address+X) into AC
                    //cout <<"C4" << endl;
                    //cout << X << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    //value contains address, add X
                    value = value + X;
                    checkPC(value, kernel_mode);
                    flag = 0; //read from mem
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    PC++;
                    break;
                case 5: //LoadIdxY value at (address+Y) into AC
                    //cout <<"C5" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    //value contains address, add Y
                    value = value + Y;
                    checkPC(value, kernel_mode);
                    flag = 0; //read from mem
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    PC++;
                    break;
                case 6: //LoadSPX load from SP + X into AC
                    //cout <<"C6" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));// reading
                    value = SP + X; //address i need value
                    checkPC(value, kernel_mode);
                    write(cpu_mem[1], &value, sizeof(value));
                    read(mem_cpu[0], &value, sizeof(value));
                    AC = value;
                    break;
                case 7: //Store addr store ac value into the address
                    //cout <<"C7" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    checkPC(value, kernel_mode);
                    //value contains address, store AC into address
                    flag = 1; //write to memory
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &value, sizeof(value)); //send index
                    write(cpu_mem[1], &AC, sizeof(AC)); // send AC to memory to be written
                    PC++;
                    break;
                case 8: //Get random number from 0 to 100
                    //cout <<"C8" << endl;
                    AC = rand() % 100 + 1;
                    break;
                case 9: // put, if port = 1 print AC int, if port = 2, print AC char
                    //cout <<"C9" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    if(value == 1){
                        cout << AC ;
                    }
                    else if(value == 2){
                        cout << (char)AC ;
                    }
                    PC++;
                    break;
                case 10: //Add X
                    //cout <<"C10" << endl;
                    AC += X;
                    break;
                case 11: //Add Y
                    //cout <<"C11" << endl;
                    AC += Y;
                    break;
                case 12: //Sub X
                    //cout <<"C12" << endl;
                    AC -= X;
                    break;
                case 13: //Sub Y
                    //cout <<"C13" << endl;
                    AC -= Y;
                    break;
                case 14: // CopyToX
                    //cout <<"C14" << endl;
                    X = AC;
                    break;
                case 15: // CopyFromX
                    //cout <<"C15" << endl;
                    AC = X;
                    break;
                case 16: //CopyToY
                    //cout <<"C16" << endl;
                    Y = AC;
                    break;
                case 17: //CopyFromY
                    //cout <<"C17" << endl;
                    AC = Y;
                    break;
                case 18: //CopytoSP
                    //cout <<"C18" << endl;
                    SP = AC;
                    break;
                case 19: // CopyFromSP
                    //cout <<"C19" << endl;
                    AC = SP;
                    break;
                case 20: // Jump address, jump to address
                    //cout <<"C20" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &PC, sizeof(PC)); //get address
                    read(mem_cpu[0], &value, sizeof(value)); //value contains address
                    checkPC(value, kernel_mode);
                    PC = value; //do i need to increment program count?
                    break;
                case 21: //Jump if equal, only if AC = 0;
                    //cout <<"C21" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag)); //read next address
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    checkPC(value, kernel_mode);
                    //check if can jump
                    if(AC == 0){
                        PC = value;
                    }
                    else{
                        PC++;
                    }
                    break;
                case 22: // Jump not equal
                    //cout <<"C22" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag)); //read next address
                    write(cpu_mem[1], &PC, sizeof(PC));
                    read(mem_cpu[0], &value, sizeof(value));
                    checkPC(value,kernel_mode);
                    if(AC != 0){
                        PC = value;
                    }
                    else{
                        PC++;
                    }
                    break;
                case 23: //Call addr, push return address to stack, jump to address
                    //cout <<"C23" << endl;
                    int jumpaddr;
                    write(cpu_mem[1], &flag, sizeof(flag)); //going to be reading
                    write(cpu_mem[1], &PC, sizeof(PC)); //get address to jump to
                    read(mem_cpu[0], &jumpaddr, sizeof(jumpaddr));
                    flag = 1;
                    write(cpu_mem[1], &flag, sizeof(flag)); //going to write
                    SP--; //decrement SP
                    write(cpu_mem[1], &SP, sizeof(SP)); //address of SP
                    PC++;
                    write(cpu_mem[1], &PC, sizeof(PC)); //push return address on stack
                    checkPC(jumpaddr,kernel_mode);
                    PC = jumpaddr;
                    //cout << jumpaddr << endl;
                    break;
                case 24: //Ret addr, pop the address off the stack, and jump to the address
                    //cout <<"C24" << endl;
                    int jumpaddr2;
                    write(cpu_mem[1], &flag, sizeof(flag)); //read from mem
                    write(cpu_mem[1], &SP, sizeof(SP)); // go to SP
                    read(mem_cpu[0], &jumpaddr2, sizeof(jumpaddr2)); //return address
                    checkPC(jumpaddr2,kernel_mode);
                    PC = jumpaddr2;
                    SP++;
                    //cout << jumpaddr2 << endl;
                    break;
                case 25: // incr X
                    //cout <<"C25" << endl;
                    X++;
                    break;
                case 26: //decr x
                    //cout <<"C26" << endl;
                    X--;
                    break;
                case 27: //Push AC onto stack
                    //cout <<"C27" << endl;
                    flag = 1;
                    write(cpu_mem[1], &flag, sizeof(flag)); //write to memory
                    SP--; //decrement SP
                    write(cpu_mem[1], &SP, sizeof(SP)); //go to SP
                    write(cpu_mem[1], &AC, sizeof(AC));
                    break;
                case 28:
                    //cout <<"C28" << endl;
                    flag = 0;
                    write(cpu_mem[1], &flag, sizeof(flag)); //read to memory
                    write(cpu_mem[1], &SP, sizeof(SP)); //go to SP
                    read(mem_cpu[0], &AC, sizeof(AC)); //return address
                    SP++;
                    break;
                case 29: //PC = 1500
                    //save current SP and PC
                    //cout <<"C29" << endl;
                    kernel_mode = true;
                    int stack;
                    int prog_count;
                    stack = SP;
                    prog_count = PC;
                    SP = 2000;
                    flag = 1;
					
                    write(cpu_mem[1], &flag, sizeof(flag));
					SP--;
					write(cpu_mem[1],&SP, sizeof(SP));
					write(cpu_mem[1], &stack, sizeof(stack));//push stack

					write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
					SP--;
					write(cpu_mem[1], &SP, sizeof(SP));
					write(cpu_mem[1], &prog_count, sizeof(prog_count)); //push PC

					write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
					SP--;
					write(cpu_mem[1], &SP, sizeof(SP));
					write(cpu_mem[1], &AC, sizeof(AC)); //push AC

					write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
					SP--;
					write(cpu_mem[1], &SP, sizeof(SP));
					write(cpu_mem[1], &IR, sizeof(IR)); //push IR

					write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
					SP--;
					write(cpu_mem[1], &SP, sizeof(SP));
					write(cpu_mem[1], &X, sizeof(X)); //push X

					write(cpu_mem[1], &flag, sizeof(flag)); //write to mem
					SP--;
					write(cpu_mem[1], &SP, sizeof(SP));
					write(cpu_mem[1], &Y, sizeof(Y)); //push Y
				   
                    PC = 1500; //jump to address 1500
                    break;
                case 30: //return from system call
                    //cout <<"C30" << endl;
                    write(cpu_mem[1], &flag, sizeof(flag)); //read from mem
                    write(cpu_mem[1], &SP, sizeof(SP)); //read from PC
                    read(mem_cpu[0], &value, sizeof(value)); //value has old PC
                    SP++;
                    Y = value;
                    //read from mem again
                    write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &SP, sizeof(SP));
                    read(mem_cpu[0], &value, sizeof(value)); //SP should be restored
                    //SP++;
                    X = value;
                    SP++;
					
					write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &SP, sizeof(SP));
                    read(mem_cpu[0], &value, sizeof(value)); //SP should be restored
                    //SP++;
                    IR = value;
                    SP++;
					
					write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &SP, sizeof(SP));
                    read(mem_cpu[0], &value, sizeof(value)); //SP should be restored
                    //SP++;
                    AC = value;
                    SP++;
					
					write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &SP, sizeof(SP));
                    read(mem_cpu[0], &value, sizeof(value)); //SP should be restored
                    //SP++;
                    PC = value;
                    SP++;
					
					write(cpu_mem[1], &flag, sizeof(flag));
                    write(cpu_mem[1], &SP, sizeof(SP));
                    read(mem_cpu[0], &value, sizeof(value)); //SP should be restored
                    //SP++;
                    SP = value;
                    kernel_mode = false;
                    break;
                case 50:
                    //cout <<"C50" << endl;
                    //kill(result, SIGKILL);
                    CPU = false; //turn cpu "off"
                    flag = 3;
                    write(cpu_mem[1], &flag, sizeof(flag));
                    exit(1);
                default:;
                    
            }
                
            //cout << "PC: " << PC << " "  << "SP: " << SP << " "<< "IR: " << IR << " " << "AC: "<<  AC << " " << "X: " << X << " " << "Y: " << Y  << endl;
            
        }
        
    }
    
}

void mem(string filename){
    //some lines may only contain a comment
    ifstream file(filename.c_str());
    int i = 0;
    if(file.is_open()){
        string line;
        while(file >> line){
            if(line.length() == 0)
                continue;
            if(line.at(0) == '.'){
                line = line.erase(0,1);
                i = atoi(line.c_str());
                continue;
            }
            if(!isdigit(line.at(0)))
            {
                
                file.ignore(sizeof(file), '\n');
                continue;
            }
            write(i,line);
            i++;
            file.ignore(sizeof(file), '\n');
        }
    }
    else{
        cout << "Error could not open file" << endl;
    }
    file.close();
}

void checkPC(int PC, bool mode){
    if(PC > 999 && mode == false){
        //cout << "Error, cannot access system memory in user mode" << endl;
        cout << "Memory violation: accessing system address " << PC << " in user mode" << endl;
        exit(1);
    }
}

int read(int address){
    return data[address];
}

void write(int address, string value){
    data[address] = atoi(value.c_str());
}
