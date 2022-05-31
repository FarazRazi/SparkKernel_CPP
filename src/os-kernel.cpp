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
#include <chrono>
#include <queue>
// files Includes
#include "./os-kernel.h"
#include "scheduler.h"

using namespace std;
using namespace chrono;
// CPU Threads
pthread_t *threadsCPU;

// Simulator mutexes
pthread_mutex_t mutexSimulator;
// for processes read write
pthread_mutex_t mutex;
pthread_cond_t no_writers;
int writers;

// Ready Process Mutexes
pthread_mutex_t mutexCurrent;
pthread_mutex_t mutexReady;
pthread_cond_t mutexReadyAdded;
// Kernel
SparkKernel Kernel;
// Queues
queue<PCB> ReadyQueue;
// Running Processes
PCB **currentProcesses;

char algo_name;
int simulatorTime = 0;
int countReady = 0, countRunning = 0, countWaiting = 0;
int total_cpus = 0, timeslice = 0;
int contextCounts = 0;
int processCreated = 0;
int processTerminated;

auto overall_time = high_resolution_clock::now();

void thread_Controller();
void simulate_cpus();
void simulateProcess(int i, PCB *cProc);
void simulateProcCreat();
void safe_usleep(long us);
void print_gantt_header();
void print_gantt_line();

// Scheduler Functions
void idle(int cpu_id);
void wake_up(PCB *process);
void preempt(int cpu_id);
void yield(int cpu_id);
void terminate(int cpu_id);

void schedular(int i);

class CPU
{
public:
    // pointer to current Process
    PCB *currentProcess;
    // 5 states of CPU
    //  1       2        3       4          5
    // IDLE - RUNNING - PREMPT - YEILD - TERMINATE
    int state;
    // thread wakeup check
    pthread_cond_t wakeup;
    // timer for context switching
    int preemptionTimer;
    CPU()
    {
    }
};

// CPU data Array
CPU *cpus_array;
void _start(int n)
{
    // cout << "\nThread #" << n + 1 << " Running" << endl;
    while (1)
    {
        pthread_mutex_lock(&mutexSimulator);
        // setting wakeup for nth cpu
        pthread_cond_signal(&cpus_array[n].wakeup);
        if (cpus_array[n].currentProcess == NULL)
        {
            // setting idle state
            cpus_array[n].state = 1;
        }
        else
        {
            // set running state
            cpus_array[n].state = 2;
            while (cpus_array[n].state == 2)
            {
                pthread_cond_wait(&cpus_array[n].wakeup, &mutexSimulator);
            }
        }

        // For IDLE STATE of CPU
        int state = cpus_array[n].state;
        pthread_mutex_unlock(&mutexSimulator);
        if (state == 1) // idle
        {
            idle(n);
        }
        else if (state == 3) // preempt
        {
            // LOCK WRITER
            pthread_mutex_lock(&mutex);
            writers++;
            pthread_mutex_unlock(&mutex);

            // call prempt
            preempt(n);

            // UNLOCK WRITER
            pthread_mutex_lock(&mutex);
            writers--;
            if (writers == 0)
            {
                pthread_cond_signal(&no_writers);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (state == 4) // YIELD
        {
            // LOCK WRITER
            pthread_mutex_lock(&mutex);
            writers++;
            pthread_mutex_unlock(&mutex);

            // call prempt
            yield(n);

            // UNLOCK WRITER
            pthread_mutex_lock(&mutex);
            writers--;
            if (writers == 0)
            {
                pthread_cond_signal(&no_writers);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if (state == 5) // TERMINATE
        {
            pthread_mutex_lock(&mutexSimulator);
            processTerminated++;
            pthread_mutex_unlock(&mutexSimulator);

            // LOCK WRITER
            pthread_mutex_lock(&mutex);
            writers++;
            pthread_mutex_unlock(&mutex);

            terminate(n);

            // UNLOCK WRITER
            pthread_mutex_lock(&mutex);
            writers--;
            if (writers == 0)
            {
                pthread_cond_signal(&no_writers);
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    // addTerminate();
    pthread_exit(0);
}
// For passing int
void *start(void *argc)
{
    _start((int)(uintptr_t)argc);
    return NULL;
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
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&no_writers, NULL);
    writers = 0;
    simulatorTime = 0;

    // Initial CPU states
    for (int i = 0; i < total_cpus; i++)
    {
        cpus_array[i].currentProcess = NULL;
        cpus_array[i].preemptionTimer = -1;
        cpus_array[i].state = 0;
        pthread_cond_init(&cpus_array[i].wakeup, NULL);
    }

    // Start CPU threads
    for (int i = 0; i < cpu_count; i++)
    {
        pthread_create(&threadsCPU[i], NULL, start, (void *)(uintptr_t)i);
        pthread_detach(threadsCPU[i]);
    }

    // Controller for threads
    // called to check for
    // simulate for cpu, I/O and
    // getting process from waiting

    thread_Controller();
}

void thread_Controller()
{
    print_gantt_header();
    while (1)
    {
        pthread_mutex_lock(&mutexSimulator);
        // Exit when all Process end
        if (processTerminated == Kernel.no_of_processes)
        {
            // printFinal();
            cout << "\nProgam ENDED\n";
            cout << contextCounts << endl;
            exit(0);
        }
        // print processes
        print_gantt_line();
        // simulate cpus
        simulate_cpus();
        // simulate i/o
        // simulate waiting-Process create
        simulateProcCreat();
        // increase Timer
        simulatorTime++;
        pthread_mutex_unlock(&mutexSimulator);
        safe_usleep(1);
    }
}
void print_gantt_header()
{
    int n;
    cout << "Time  Ru Re Wa     ";
    for (n = 0; n < total_cpus; n++)
        cout << " CPU " << n << "    ";
    cout << "\t";
}
void print_gantt_line()
{
    unsigned int current_ready = 0, current_running = 0, current_waiting = 0;
    unsigned int n;

    /*
     * Update number of processes in each state.
     */
    pthread_mutex_lock(&mutex);
    while (writers > 0)
    {
        pthread_cond_wait(&no_writers, &mutex);
    }
    for (n = 0; n < Kernel.no_of_processes; n++)
    {
        switch (Kernel.pcb_array[n].state)
        {
        case 2:
            current_ready++;
            countReady++;
            break;

        case 3:
            current_running++;
            countRunning++;
            break;

        case 4:
            current_waiting++;
            countWaiting++;
            break;

        default:
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    /* Print time */
    cout << simulatorTime / 10.0 << "\t" << current_running << " " << current_ready << " " << current_waiting;

    /* Print running processes */
    for (n = 0; n < total_cpus; n++)
    {
        if (cpus_array[n].currentProcess != NULL)
            cout << "\t" << cpus_array[n].currentProcess->process_name;
        else
            cout << "\t(IDLE)  ";
    }
    cout << "\n";
    /* Print I/O requests */
    // printf("     <");
    // r = io_queue_head;
    // while (r != NULL)
    // {
    //     printf(" %s", r->pcb->name);
    //     r = r->next;
    // }
    // printf(" <\n");
}
void simulate_cpus()
{
    for (int i = 0; i < total_cpus; i++)
    {
        if (cpus_array[i].currentProcess != NULL)
            // simulate Process
            simulateProcess(i, cpus_array[i].currentProcess);
    }
}
void simulateProcess(int i, PCB *cProc)
{
    // cProc - PCB - state
    char process_type = cProc->process_type;
    float time = cProc->process_cpu_time;

    if (time > 0) // needs to run
    {
        time--;
        cProc->process_cpu_time = time;
        // For stopping a process
        cpus_array[i].preemptionTimer--;
        if (cpus_array[i].preemptionTimer == 0)
        {
            cpus_array[i].state = 3;
            pthread_cond_signal(&cpus_array[i].wakeup);
            pthread_cond_wait(&cpus_array[i].wakeup, &mutexSimulator);
        }
    }
    else
    {
        cout << "here" << endl;
        cpus_array[i].state = 5;
        pthread_cond_signal(&cpus_array[i].wakeup);

        pthread_cond_wait(&cpus_array[i].wakeup, &mutexSimulator);
    }
    // if (process_type = 'I') // needs I/O
    // {
    //     // submit for I/O
    //     // submitIO(cProc, cProc->process_IO_time);

    //     // yield call on CPU
    //     cpus_array[i].state = 4;
    //     pthread_cond_signal(&cpus_array[i].wakeup);

    //     pthread_cond_wait(&cpus_array[i].wakeup, &mutexSimulator);
    // }

    // Running
    //  burst check
    //  true -- still running decriment remaining time
    //  false -- completed running
    //   I/O or Terminate
    // wait for I/O
    // Terminate
}

// this function wakeUp Processes
void simulateProcCreat()
{
    if (processCreated < Kernel.no_of_processes && Kernel.pcb_array[processCreated].process_arival_time <= simulatorTime / 10.0)
    {
        pthread_mutex_unlock(&mutexSimulator);
        wake_up(&Kernel.pcb_array[processCreated]);
        pthread_mutex_lock(&mutexSimulator);
        processCreated++;
    }
}

// void initialize_ready()
// {

//     auto start_time = high_resolution_clock::now();
//     for (int i = 0; i < Kernel.no_of_processes; i++)
//     {
//         auto curr_time = high_resolution_clock::now();
//         auto check = duration_cast<microseconds>(curr_time - start_time);
//         if (check.count() >= Kernel.pcb_array[i].process_arival_time * 1000000)
//         {
//             ReadyQueue.push(Kernel.pcb_array[i]);
//             // pthread_mutex_lock(&mutexSimulator);
//             cout << "\nProcess arrived " << Kernel.pcb_array[i].process_name << " at " << check.count();
//             // pthread_mutex_unlock(&mutexSimulator);
//         }
//         else
//         {
//             i--;
//         }
//     }
// }

int main(int argc, char *argv[])
{
    string inputfile = argv[1], outputfile = argv[5], ch = argv[3];
    algo_name = ch[0];
    total_cpus = stoi(argv[2]);
    timeslice = stoi(argv[4]);

    Kernel.file_handling(inputfile);
    // initialize_ready();
    //  Kernel.show_pcb();

    pthread_mutex_init(&mutexReady, NULL);
    pthread_cond_init(&mutexReadyAdded, NULL);

    currentProcesses = new PCB *[total_cpus];
    pthread_mutex_init(&mutexCurrent, NULL);

    start_simulator(total_cpus);

    return 0;
}
void safe_usleep(long us)
{
    struct timespec ts;
    ts.tv_sec = us / 1000000ll;
    ts.tv_nsec = (us % 1000000ll) * 1000ll;

    while (nanosleep(&ts, &ts) != 0)
        ;
}
void context_switch(int i, PCB *cProc, int pTime)
{
    // assert(i < total_cpus);
    //   assert(cProc == NULL || (cProc >= Kernel.pcb_array && cProc <= Kernel.pcb_array + Kernel.no_of_processes - 1));

    contextCounts++;

    pthread_mutex_lock(&mutex);
    writers++;
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutexSimulator);

    cpus_array[i].currentProcess = cProc;

    cpus_array[i].preemptionTimer = pTime;

    pthread_mutex_unlock(&mutexSimulator);

    pthread_mutex_lock(&mutex);
    writers--;
    if (writers == 0)
        pthread_cond_signal(&no_writers);
    pthread_mutex_unlock(&mutex);
}
void schedular(int i)
{
    PCB *curr = NULL;
    pthread_mutex_lock(&mutexReady);
    if (!ReadyQueue.empty())
    {
        if (algo_name == 'p')
        {
            // For priority algo change ready Queue
        }
        else
        {
            curr = &ReadyQueue.front();
            ReadyQueue.pop();
        }
        // set state ready
        curr->state = 3;
    }
    pthread_mutex_unlock(&mutexReady);

    pthread_mutex_lock(&mutexCurrent);
    currentProcesses[i] = curr;
    pthread_mutex_unlock(&mutexCurrent);
    if ((algo_name == 'r') && curr != NULL)
        context_switch(i, curr, timeslice);
    else
        context_switch(i, curr, -1);
}
void wake_up(PCB *process)
{
    pthread_mutex_lock(&mutexReady);
    // process ready
    process->state = 2;
    ReadyQueue.push(*process);

    pthread_cond_signal(&mutexReadyAdded);
    pthread_mutex_unlock(&mutexReady);
    // for priority based set accordingly
}
void idle(int i)
{
    pthread_mutex_lock(&mutexReady);
    // if no process set cpu to idle
    if (ReadyQueue.empty())
        // wait for a process to be added
        pthread_cond_wait(&mutexReadyAdded, &mutexReady);
    pthread_mutex_unlock(&mutexReady);
    // call schedular
    schedular(i);
}
void preempt(int i)
{
    pthread_mutex_lock(&mutexCurrent);
    // set process to ready
    currentProcesses[i]->state = 2;
    // add to ready queue
    ReadyQueue.push(*currentProcesses[i]);
    // signal of adding to queue
    pthread_cond_signal(&mutexReadyAdded);
    // unlock for ready queue
    pthread_mutex_unlock(&mutexReady);
    // unlock for current Processes array
    pthread_mutex_unlock(&mutexCurrent);

    schedular(i);
}
void yield(int i)
{
    pthread_mutex_lock(&mutexCurrent);
    // set process to waiting
    currentProcesses[i]->state = 4;
    pthread_mutex_unlock(&mutexCurrent);

    schedular(i);
}
void terminate(int i)
{
    pthread_mutex_lock(&mutexCurrent);
    // set process to Termination
    currentProcesses[i]->state = 5;
    pthread_mutex_unlock(&mutexCurrent);

    schedular(i);
}
