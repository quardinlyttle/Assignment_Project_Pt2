/* Family Name: Lyttle
Given Name(s): Quardin
Student Number: 215957889
EECS Login ID: lyttleqd
YorkU email address: lyttleqd@my.yorku.ca
*/

#include "sch-helpers.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define NUMofCores 4

process_queue readyQ;
process_queue DeviceQ;
process_queue completedQ;
process *cpu[NUMofCores];
process *sortingQ[MAX_PROCESSES];
process processes[MAX_PROCESSES+1];   

int numberOfProcesses;  
int cpuActiveTime;
int simulationtime;
int nextProcess;
int quantumTime;
int numContextSwitch;
int sortingProcessNum;
bool systemrunning;


/*Initilaize all the Global varialbles */
void initializeGlobals(void){

    initializeProcessQueue(&readyQ);
    initializeProcessQueue(&DeviceQ);
    initializeProcessQueue(&completedQ);
    
    for(int i =0; i<NUMofCores; i++){
        cpu[i]=NULL;
    }

    cpuActiveTime=0;
    simulationtime=0;
    nextProcess=0;
    numContextSwitch=0;
    sortingProcessNum=0;

    systemrunning=true;
}

/*CPU Scheduler.*/
void cpuOperation(void){

    for(int i=0;i<NUMofCores;i++){

        /*if the CPU isn't busy, pull from the readyQ if available*/
        if(cpu[i]==NULL){
            if(readyQ.front != NULL){
                cpu[i]=readyQ.front->data;
                printf("about to deque from readyq of size %d\n",readyQ.size);
                //sleep(2);
                dequeueProcess(&readyQ);
                if(cpu[i]->startTime < cpu[i]->arrivalTime ){
                    cpu[i]->startTime=simulationtime;
                    cpu[i]->waitingTime=cpu[i]->startTime-cpu[i]->arrivalTime;
                }
                cpu[i]->quantumRemaining=quantumTime;
            }
        }

        /*If the CPU is busy, continue processing*/
        if(cpu[i]!= NULL){
            cpuActiveTime++;
            cpu[i]->quantumRemaining--;
            cpu[i]->bursts[cpu[i]->currentBurst].step++;

            /*When the current burst is finished, move to IO if more bursts, end if not*/
            if(cpu[i]->bursts[cpu[i]->currentBurst].step == cpu[i]->bursts[cpu[i]->currentBurst].length){
                printf("burst completed on CPU %d for pid %d. Current burst: %d. Bursts left: %d.\n",i,cpu[i]->pid,cpu[i]->currentBurst, cpu[i]->numberOfBursts);
                cpu[i]->quantumRemaining=0;
                if(cpu[i]->currentBurst < cpu[i]->numberOfBursts){
                    enqueueProcess(&DeviceQ,cpu[i]);
                    cpu[i]->currentBurst++;
                    cpu[i]=NULL;
                    printf("Moved to DeviceQ from CPU %d\n",i);
                    continue;
                }
                else if(cpu[i]->currentBurst==cpu[i]->numberOfBursts){
                    cpu[i]->endTime=simulationtime;
                    enqueueProcess(&completedQ,cpu[i]);
                    cpu[i]=NULL;
                    continue;
                    }                
            }
            
            /*perform context switch if quantum is up and process is not finished*/
            if(cpu[i]->quantumRemaining==0){
                if(readyQ.front!=NULL){
                    printf("Doing Context switch on CPU%d for process%d\n",i,cpu[i]->pid);
                    sortingQ[sortingProcessNum]=cpu[i];
                    sortingProcessNum++;
                    numContextSwitch++;
                    cpu[i]=NULL;
                }
                else if(readyQ.front==NULL){
                    cpu[i]->quantumRemaining=quantumTime;
                }

            }

        }
    }

}

/*Sort processes with the same scheduling criteria*/
void sorter(void){
    qsort(sortingQ,sortingProcessNum,sizeof(process*),compareByPID);
    if(sortingProcessNum>0){
            printf("Current amount of sorting processes %d\n",sortingProcessNum);
    }

    for(int i=0; i<sortingProcessNum;i++){
        enqueueProcess(&readyQ,sortingQ[i]);
        printf("Process %d has been sorted and added to readyQ.  Size is now %d\n",sortingQ[i]->pid,readyQ.size);
        //sleep(2);
    }
    sortingProcessNum=0;
}

/*Updating the processes in the Device IO/waiting and ensure they are stepping through time.*/
void updateDeviceQ(void){
    int size = DeviceQ.size;

    for(int i=0;i<size;i++){
        process *p=DeviceQ.front->data;
        dequeueProcess(&DeviceQ);
        p->bursts[p->currentBurst].step++;
        if(p->bursts[p->currentBurst].step== p->bursts[p->currentBurst].length){
            sortingQ[sortingProcessNum]=p;
            sortingProcessNum++;
            p->currentBurst++;
            continue;
        }
        else{
            enqueueProcess(&DeviceQ,p);
        }

    }
}

void updateReadyQ(void){
    int size = readyQ.size;
    printf("Updating readyQ of size %d\n",size);
    for(int i=0;i<size;i++){
    process *p=readyQ.front->data;
    dequeueProcess(&readyQ);
    p->waitingTime++;
    enqueueProcess(&readyQ,p);
    printf("Process %d updated\n",p->pid);

    }
}


int main(int argc, char* argv[])
{
    /*Check if the correct time quantum arguments are passed through*/
    if(argc <= 1 || argc > 2){
        error_bad_quantum();
    }

    /*Initialize quantum time to provided parameter*/
    quantumTime = atoi(argv[1]);

    /*get all the processes and sort them by arrival time and Process ID if arrival time the same*/
    int status;

    while ((status=readProcess(&processes[numberOfProcesses]))!=0)  {
         if(status==1)  numberOfProcesses ++;
         printf("Process %d added to array\n",processes[numberOfProcesses-1].pid);
    } 
    qsort(processes, numberOfProcesses, sizeof(process), compareByArrivalandPID);

    /*Initialize all the Processes*/
    initializeGlobals();
    printf("Initialized Globals Succesfully\n");


    /******SIMULATION START******/
    while(systemrunning){
        printf("in system time stamp:%d\n",simulationtime);
        printf("Initial check for ready.size: %d\n",readyQ.size);

        /*check if there are processes arriving at this current time and add them. They have already been sorted by PID if the arrivalTime is the same*/
        while(processes[nextProcess].arrivalTime == simulationtime){
            enqueueProcess(&readyQ,&processes[nextProcess]);  
            nextProcess++;  
            printf("process added to readyQ from arrivals and is now size %d\n",readyQ.size); 
                printf("next arrival at%d\n",processes[nextProcess].arrivalTime);     
        }

        sorter();//add any waiting processes that needed to be sorted to the readyQ
        cpuOperation();//Conduct the CPU Scheduler Operations as necessary
        updateDeviceQ();//Update the steps in device Q and move them to sorting if done.
        updateReadyQ();//Update waiting time for processes


       printf("nextProcess: %d, numberOfProcesses: %d, readyQ.front: %p, readyQ.size: %d, DeviceQ.front: %p DeviceQ.size: %d, completedQ.size %d\n",
       nextProcess, numberOfProcesses, (void *)readyQ.front, readyQ.size, (void *)DeviceQ.front, DeviceQ.size, completedQ.size);

       if( nextProcess >= numberOfProcesses && readyQ.size ==0 && DeviceQ.size==0){
            printf("entered if\n");
            systemrunning=false;
            break;
        }

        simulationtime++;


    }

    int totalWaitTime=0;
    int totalTurnAroundTime=0;        
    int lastpid =0;

    for (int i=0; i<numberOfProcesses; i++){
        totalWaitTime += processes[i].waitingTime;
        totalTurnAroundTime += (processes[i].endTime-processes[i].arrivalTime);
        if(processes[i].endTime==simulationtime){
            lastpid=processes[i].pid;
        }
    }
    double avgWait = totalWaitTime/ (double) numberOfProcesses;
    double avgTurnAround = totalTurnAroundTime/ (double) numberOfProcesses;
    double avgCPU=  (cpuActiveTime/simulationtime) *100.00;
    printf("This is your cpuActiveTime %d\n",cpuActiveTime);



    printf("Average waiting time:\t%.2f milliseconds\n"
            "Average turnaround time:\t%.2f milliseconds\n",avgWait,avgTurnAround);
    printf("Time all processes finished:\t%d milliseconds\n",simulationtime);
    printf("Average CPU Utilization:\t%.2f%%\n"
            "Number of context switches:\t%d\n",avgCPU,numContextSwitch);
    printf("PID of the last process to finish:\t%d\n",lastpid);

    return 0;

}