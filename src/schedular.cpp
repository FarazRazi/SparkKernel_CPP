#include <iostream>
#include "scheduler.h"
using namespace std;

void Scheduler()
{
    cout << "Hello World" << endl;
}

void *start(void *argc)
{
    
    int n = (int)(uintptr_t)argc;
    cout << "Thread #" << n + 1 << " Running" << endl;
    // Scheduler();
    pthread_exit(0);
}