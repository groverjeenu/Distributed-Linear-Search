// Assignment 1
// Q1. Distributed Linear Search

// Objective
// In this assignment, we will be implementing Distributed Linear Search(DLS) using the concepts Linux System calls.
// In DLS, the array is split up into subarrays and searching the sub-arrays in delegated to other processes (hence distributed).

// Group Details
// Member 1: Jeenu Grover (13CS30042)
// Member 2: Ashish Sharma (13CS30043)

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAXN 1000000
int a[MAXN];
pid_t mpid;

void sigusr1(int signo, siginfo_t *info, void *extra)
{
    int int_val = info->si_value.sival_int;
    printf("Index of entered number is : %d\n",int_val);
    killpg(mpid,SIGTERM);
    exit(0);
}


// Function for performing the Distributed Linear Search
// Input: l-left index, h-right index, n-Number to search, len-Length of the original array
void dls(int l ,int h , int n,int len)
{
    int signals = 0,i;
    union sigval value;
    int idx;

    // Base Case: If Length of the segment to search for is less than = 5
    if((h-l+1) <= 5)
    {
        // Search for the number in the segment
        for(i=l; i<=h; i++)
        {
            if(a[i] == n)
            {
                idx = i;
                signals = 5;
                // Number found --- send a signal to kill all the processes
                value.sival_int = idx;
                if(sigqueue(mpid, SIGUSR2, value) == 0)
                {
                    //printf("signal sent successfully!!\n");
                }
                break;
            }
        }
        // If this was the only process created
        if(signals==0 && (h-l+1) == len) printf("Number entered was not found in the given array\n");

    }
    else
    {
        // Create 2 new child processes and distribute the array to them

        // Create Child Process 1
        pid_t pid1 = fork();

        if(pid1 == 0)
        {
            // In Child Process 1
            // Search for the number in the first half
            setpgid(getpid(), mpid);
            dls(l,(l+h)/2,n,len);
            exit(0);
        }

        else
        {
            // Create Child Process 2
            pid_t pid2 = fork();
            if(pid2 == 0)
            {
                // In Child Process 1
                // Search for the number in the second half
                setpgid(getpid(), mpid);
                dls(((l+h)/2)+1,h,n,len);
                exit(0);
            }
            else
            {
                // In Parent Process
                int status1 = 0,status2 = 0;
                // Print pid and the array under consideration
                printf("Process is %d, l = %d, h = %d\n",getpid(),l,h );

                // Wait for Child Process 1 to finish --- Status in status1
                waitpid(pid1,&status1,0);
                // Wait for Child Process 2 to finish --- Status in status2
                waitpid(pid2,&status2,0);
                // If the child exited normally and parent is the main process, print not found
                if((h-l+1) == len && WIFSIGNALED(status1) == 0 && WIFSIGNALED(status2) == 0) printf("Number entered was not found in the array\n");



            }
        }
    }

}

int main()
{
    char filename[256];	// File containing tab separated integers to search from
    int n;	// Integer to search
    printf("Enter Filename: ");
    scanf("%s",filename);
    printf("Enter number to search for: ");
    scanf("%d",&n);
    FILE * fp = fopen(filename,"r");	// Open the input file
    int len = 0;

    while(fscanf(fp,"%d",&a[len])!= EOF)
    {
        len++;
    }

    fclose(fp);
    // Set mpid (main process id)
    mpid = getpid();


    // Install Signal Handler

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &sigusr1;

    if (sigaction(SIGUSR2, &action, NULL) == -1)
    {
        perror("sigusr: sigaction");
        return 0;
    }

    //printf("Signal handler installed, waiting for signal\n");

    dls(0,len-1,n,len);	// Call dls to search n

    return 0;

}

