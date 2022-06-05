#include <string>
#include <string.h>
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
    int state;
    int process_id;
    string process_name;
    int process_priority;
    float process_arival_time;
    char process_type;
    float process_cpu_time;
    int process_IO_time;
    int process_IO_count;
    int *process_IO_stamps;
    int process_fixed_time;

    PCB()
    {
        process_id = 0;
        process_name = "";
        process_priority = 0;
        process_arival_time = 0.0;
        process_cpu_time = 0.0;
        process_IO_count = 0;
    }
    PCB(int id, string name, int priority, int a_time, char type, int cpu_time, int io_time)
    {
        process_id = id;
        process_name = name;
        process_priority = priority;
        process_arival_time = a_time;
        process_type = type;
        process_cpu_time = cpu_time;
        process_IO_count = io_time;
    }
};

class SparkKernel
{
private:
    /* data */
public:
    PCB *pcb_array;
    int no_of_processes;
    SparkKernel()
    {
        pcb_array = NULL;
        no_of_processes = 0;
    }

    void count_no_of_processes(string inputfile)
    {
        ifstream file(inputfile);
        string line = "";
        getline(file, line);
        while (getline(file, line))
        {
            no_of_processes++;
        }
        file.close();
    }

    void file_handling(string inputfile)
    {
        count_no_of_processes(inputfile);
        pcb_array = new PCB[no_of_processes];

        ifstream file(inputfile);
        string line = "", str = "";
        char chr;
        int k = 0, p = 0;
        getline(file, line);
        while (!file.eof())
        {

            file >> pcb_array[k].process_name;
            file >> str;
            p = stoi(str);
            pcb_array[k].process_priority = p;
            file >> str;
            p = stoi(str);
            pcb_array[k].process_arival_time = p;
            file >> str;
            pcb_array[k].process_type = str[0];
            file >> str;
            p = stoi(str);
            pcb_array[k].process_cpu_time = p;
            file >> str;
            p = stoi(str);
            pcb_array[k].process_IO_count = p;
            pcb_array[k].process_fixed_time = pcb_array[k].process_cpu_time;
            if (pcb_array[k].process_type == 'I')
                setIOTimes(pcb_array[k]);

            k++;
        }
    }
    void setIOTimes(PCB &pcb)
    {
        srand(time(NULL));
        const int n = pcb.process_IO_count;
        int n2;
        pcb.process_IO_stamps = new int[n];

        for (int i = 0; i < n; i++)
        {
            n2 = pcb.process_cpu_time;
            n2 = rand() % n2 + 1;
            pcb.process_IO_stamps[i] = n2;
        }
        int i, j, min, temp;
        for (i = 0; i < n - 1; i++)
        {
            min = i;
            for (j = i + 1; j < n; j++)
                if (pcb.process_IO_stamps[j] > pcb.process_IO_stamps[min])
                    min = j;
            temp = pcb.process_IO_stamps[i];
            pcb.process_IO_stamps[i] = pcb.process_IO_stamps[min];
            pcb.process_IO_stamps[min] = temp;
        }

        for (int i = 0; i < n; i++)
        {
            cout << pcb.process_IO_stamps[i] << " ";
        }
        cout << "\n";
    }

    void show_pcb()
    {
        cout << "PROCNAME\tPRIORITY\tARRIVAL-TIME\tPROC-TYPE\tCPU-TIME\tI/O-TIME" << endl;
        for (int i = 0; i < no_of_processes; i++)
        {
            cout << pcb_array[i].process_name << "\t";
            // For Row Alignment
            if (pcb_array[i].process_name.length() < 8)
                cout << "\t";
            cout << pcb_array[i].process_priority << "\t\t";
            cout << pcb_array[i].process_arival_time << "\t\t";
            cout << pcb_array[i].process_type << "\t\t";
            cout << pcb_array[i].process_cpu_time << "\t\t";
            cout << pcb_array[i].process_IO_count << "\n";
        }
    }
};