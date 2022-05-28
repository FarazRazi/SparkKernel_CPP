#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include<fstream>
#include "./os-kernel.h"

using namespace std;
int main(int argc, char *argv[])
{

    string inputfile = argv[1],outputfile = argv[5],ch = argv[3]; 
    char algo_name = ch[0];
    int cpus = stoi(argv[2]);
    int timeslice = stoi(argv[4]);
    
   // file reading 
   ifstream file(inputfile);
   string line="",str="";
   char chr;
   getline(file,line);
   PCB *PCB_array;
   int process_id;
   string process_name;
   int process_priority;
   int process_arival_time;
   char process_type;
   int process_cpu_time;
   int process_IO_time;
   while(getline(file,line))
   {
       //cout<<line<<"\n";
       str="";
       for(int i=0;line[i]!='\0';i++)
       {
           chr=line[i];
           if(chr != '\t')
           {
               
               str += chr;
           }
           else
           {
               str += ' ';
           }
       }
        //cout<<"\n"<<str<<"\n";
       
   }
 
    
    return 0;
}
