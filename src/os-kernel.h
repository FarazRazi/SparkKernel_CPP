class Process
{
public:
    // 5 states of process
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
    PCB(/* args */);
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
