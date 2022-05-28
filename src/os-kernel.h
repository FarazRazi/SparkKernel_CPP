#include<string>
using namespace std;
class Process
{
public:
    // 5 states of process
    //  1       2        3       4          5
    // NEW - READY - RUNNING - WAITING - TERMINATED
    int P_state;
    // process id
    int P_id;

    Process()
    {
        P_state = 0;
    }
    Process(int pid)
    {
        P_state = 0;
        P_id = pid;
    }
};
class PCB
{
private:
    /* data */
public:
    int process_id;
    string process_name;
    int process_priority;
    int process_arival_time;
    char process_type;
    int process_cpu_time;
    int process_IO_time;
    PCB(int id,string name,int priority,int a_time,char type,int cpu_time,int io_time)
    {
        process_id=id;
        process_name=name;
        process_priority=priority;
        process_arival_time=a_time;
        process_type=type;
        process_cpu_time=cpu_time;
        process_IO_time=io_time;
    }
};

class SparkKernel
{
private:
    /* data */
public:
    SparkKernel(/* args */);
    // The process is being created and has not yet begun executing.
    void newProcess()
    {
    }
};
