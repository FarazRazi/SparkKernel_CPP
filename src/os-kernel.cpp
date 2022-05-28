#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string>
#include<fstream>
#include "./os-kernel.h"

using namespace std;
int main(int argc, char *argv[])
{

    string inputfile = argv[1],outputfile = argv[5],ch = argv[3];
    
    char algo_name = ch[0];
    int cpus = stoi(argv[2]);
    int timeslice = stoi(argv[4]);
     
   // cout<<"\n"<<inputfile<<"\n"<<cpus<<"\n"<<algo_name<<"\n"<<timeslice<<"\n"<<outputfile<<"\n";
    
   // file reading 
   ifstream file(inputfile);
   string line="";
   getline(file,line);

   while(getline(file,line))
   {
       cout<<line<<"\n";
   }
    
    
    return 0;
}
