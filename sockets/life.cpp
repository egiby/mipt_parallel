#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;

#define ADDR "127.0.0.1"
#define PORT "50100"

int num_ready(0);
int num_workers(0);

bool is_ready()
{
    return num_ready == num_workers;
}

int main(int argc, char ** argv)
{
    if (argc != 3)
    {
        cerr << "it is not good!\n";
        return 1;
    }
    
    num_workers = atoi(argv[2]);
    
    vector<pid_t> pids(num_workers);
    
    for (int i = 0; i < num_workers; ++i)
    {
        int pid = fork();
        if (pid == 0)
        {
            char id[10];
            memset(id, 0, 10);
            sprintf(id, "%d", i);
            execl("./client", "./client", argv[1], argv[2], id, ADDR, PORT, NULL);
            if (errno != 0)
                perror("execl");
            return 0;
        }
        else
            pids[i] = pid;
    }
    
    //~ printf("what is happened?\n");
    
    for (int i = 0; i < num_workers; ++i)
    {
        //~ printf("%d\n", i);
        int wstatus;
        do
        {
            int w = waitpid(pids[i], &wstatus, 0);
            if (w < 0)
            {
                //~ perror("waitpid");
                errno = 0;
                break;
            }
            if (!WIFEXITED(wstatus))
                usleep(100);
        } while (!WIFEXITED(wstatus));
    }
    
    return 0;
}
