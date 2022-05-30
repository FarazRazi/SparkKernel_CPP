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
// files Includes
#include "./os-kernel.h"
#include "scheduler.h"

using namespace std;

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

// CPU Threads
pthread_t *threadsCPU;
//  mutexes
pthread_mutex_t mutexSimulator;
// CPU data Array
CPU *cpus_array;

int simulatorTime = 0;
int countReady = 0, countRunning = 0, countWaiting = 0;

int total_cpus = 0, timeslice = 0;

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
    }
    for (int i = 0; i < cpu_count; i++)
    {
        pthread_join(threadsCPU[i], NULL);
    }

    // Controller for threads
    // called to check for
    // simulate for cpu, I/O and
    // getting process from waiting

    // thread_Controller();
}
void thread_Controller()
{
    while (1)
    {
        pthread_mutex_lock(&mutexSimulator);
        // Exit when all Process end
        // if ()
        // {
        //     printFinal();
        //     exit(0);
        // }

        // simulate cpus
        // simulate i/o
        // simulate waiting-Process
        // increase Timer
        pthread_mutex_unlock(&mutexSimulator);
    }
}
int main(int argc, char *argv[])
{
    string inputfile = argv[1], outputfile = argv[5], ch = argv[3];
    char algo_name = ch[0];
    total_cpus = stoi(argv[2]);
    timeslice = stoi(argv[4]);

    SparkKernel Kernel;
    Kernel.file_handling(inputfile);
    // Kernel.show_pcb();
    start_simulator(total_cpus);
    return 0;
}
