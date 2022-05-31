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


int total_cpus=0,timeslice=0;

int *cpus_array;
int *executing_array;



void* start(void *argc)
{
      
      pthread_exit(0);
}
int main(int argc, char *argv[])
{

    string inputfile = argv[1],outputfile = argv[5],ch = argv[3]; 
    char algo_name = ch[0];
    total_cpus = stoi(argv[2]);
    timeslice = stoi(argv[4]);
   
    SparkKernel Kernel;
    Kernel.file_handling(inputfile);
    Kernel.show_pcb();




    pthread_t tid_array[total_cpus];
    cpus_array= new int[total_cpus];
    executing_array= new int[total_cpus];
    
    for(int i=0;i<total_cpus;i++)
    {
        int *j= new int;
        *j=i+1;
        pthread_create(&tid_array[i],NULL,start,&j);
        pthread_detach(tid_array[i]);
    }
   
  
   
 
    
    return 0;
}
