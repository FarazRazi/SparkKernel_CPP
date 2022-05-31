#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include <fstream>
#include<chrono>
#include<queue>
// files Includes
#include "./os-kernel.h"
#include "scheduler.h"

using namespace std;
using namespace chrono;
// CPU Threads
pthread_t *threadsCPU;
//  mutexes
pthread_mutex_t mutexSimulator;
//Kernel
SparkKernel Kernel;
//Queues
queue<PCB> ReadyQueue;


int simulatorTime = 0;
int countReady = 0, countRunning = 0, countWaiting = 0;
int total_cpus = 0, timeslice = 0;
int processCreated=0;
int processTerminated;

auto overall_time= high_resolution_clock::now();

void addTerminate();
void thread_Controller();
void simulate_cpus();
void simulateProcess();
void simulateProcCreat();
void wake_up();

class CPU
{
public:
    // pointer to current Process
    PCB *currentProcess;
    // 5 states of CPU
    //  1       2        3       4          5
    // IDLE - RUNNING - PREMPT - YEILD - TERMINATE
    int state;
    CPU()
    {
    }
};

// CPU data Array
CPU *cpus_array;

void *start(void *threadargc)
{
    int n = (int)(uintptr_t)threadargc;
    cout << "\nThread #" << n + 1 << " Running" << endl;

    addTerminate();
    pthread_exit(0);
}

void start_simulator(int cpu_count)
{
    cout << "Threads Count: " << cpu_count << endl;
    if (cpu_count < 1 || cpu_count > 4)
    {
        cout << ("CPU Count must be from 1 to 4!\n");
        exit(-1);
    }

    threadsCPU = new pthread_t[total_cpus];
    cpus_array = new CPU[total_cpus];

    // Initiallize Mutexes and timer
    pthread_mutex_init(&mutexSimulator, NULL);
    simulatorTime = 0;
    // Initial CPU states
    for (int i = 0; i < total_cpus; i++)
    {
        cpus_array[i].currentProcess = NULL;
        cpus_array[i].state = 0;
    }

    // Start CPU threads
    for (int i = 0; i < cpu_count; i++)
    {
        pthread_create(&threadsCPU[i], NULL, start, (void *)(uintptr_t)i);
        //pthread_detach(threadsCPU[i]);
    }
    // for(int i=0;i<cpu_count;i++)
    // {
    //     pthread_join(threadsCPU[i],NULL);
    // }

    // Controller for threads
    // called to check for
    // simulate for cpu, I/O and
    // getting process from waiting

     thread_Controller();
}

void thread_Controller()
{
    while (1)
    {
        pthread_mutex_lock(&mutexSimulator);
        //Exit when all Process end
        if (processTerminated == Kernel.no_of_processes)
        {
            //printFinal();
            exit(0);
        }

        // simulate cpus 
        simulate_cpus();
        // simulate i/o

        // simulate waiting-Process create 
        simulateProcCreat();
        // increase Timer
        
        // P1 - P2 - P3
        
        pthread_mutex_unlock(&mutexSimulator);
    }
}

void addTerminate()
{
    processTerminated++;
}
void simulate_cpus()
{
    for (int i=0;i<total_cpus;i++)
    {
        if(cpus_array[i].currentProcess!=NULL)
            //simulate Process
            simulateProcess(i,cpus_array[i].currentProcess);
    }
}
void simulateProcess(int i,PCB * cProc)
{
    //cProc - PCB - state 

    // Running 
    //  burst check
    //  true -- still running decriment remaining time
    //  false -- completed running
    //   I/O or Terminate
    // wait for I/O
    // Terminate
}

// this function wakeUp Processes
void simulateProcCreat(){
    if(processCreated<Kernel.no_of_processes)
    {
        pthread_mutex_unlock(&mutexSimulator);
        wake_up(&Kernel.pcb_array[processCreated]);
        pthread_mutex_lock(&mutexSimulator);
        processCreated++;
    }
}
void wake_up(PCB *process){

    process->state=1; //ready
    ReadyQueue.push(*process);
}

void initialize_ready()
{

    auto start_time= high_resolution_clock::now();
    for(int i=0;i<Kernel.no_of_processes;i++)
    {
        auto curr_time= high_resolution_clock::now();   
        auto check=duration_cast<microseconds>(curr_time-start_time);
        if(check.count()>=Kernel.pcb_array[i].process_arival_time*1000000)
        {
            ReadyQueue.push(Kernel.pcb_array[i]);
           // pthread_mutex_lock(&mutexSimulator);
            cout<<"\nProcess arrived "<<Kernel.pcb_array[i].process_name<<" at "<<check.count();
           // pthread_mutex_unlock(&mutexSimulator);
        } 
        else
        {
            i--;
        }
    }
}

int main(int argc, char *argv[])
{
    string inputfile = argv[1], outputfile = argv[5], ch = argv[3];
    char algo_name = ch[0];
    total_cpus = stoi(argv[2]);
    timeslice = stoi(argv[4]);

    
    Kernel.file_handling(inputfile);
    //initialize_ready();
    // Kernel.show_pcb();
    start_simulator(total_cpus);

    return 0;
}
