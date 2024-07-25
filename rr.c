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
process *cpu[NUMofCores];
process *sortingQ[MAX_PROCESSES];

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
    
    for(int i =0; i<NUMofCores; i++){
        cpu[i]=NULL;
    }

    numberOfProcesses=0
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
            if(readyQ->front != NULL){
                cpu[i]=readyQ.front->data;
                dequeueProcess(&readyQ);
                if(cpu.startTime == 0 && cpu.bursts[0].step==0 ){
                    cpu[i]->startTime=simulationtime;
                    cpu[i]->waitingTime=cpu[i]->startTime-cpu[i]->arrivalTime;
                }
                cpu.quantumRemaining=quantumTime;
            }
        }

        /*If the CPU is busy, continue processing*/
        if(cpu[i]!= NULL){
             
            cpuActiveTime++;
            cpu[i]->quantumRemaining--;
            cpu[i]->bursts[cpu[i]->currentBurst].step++;

            /*When the current burst is finished, move to IO if more bursts, end if not*/
            if(cpu[i]->bursts[cpu[i]->currentBurst].step == cpu[i]->bursts[cpu[i]->currentBurst].length){
                cpu[i]->quantumRemaining=0;
                if(cpu[i]->currentBurst<cpu[i]->numberOfBursts){
                    enqueueProcess(&DeviceQ,cpu[i]);
                    cpu[i]->currentBurst++;
                    cpu[i]=NULL;
                    continue;
                }
                else if(cpu[i]->currentBurst==cpu[i]->numberOfBursts){
                    cpu[i]->endTime=simulationtime;
                    cpu[i]=NULL;
                    continue;
                    }                
            }

            /*perform context switch if quantum is up and process is not finished*/
            if(cpu[i]->quantumRemaining==0){
                if(readyQ.front!=NULL){
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
    for(int i=0; i<sortingProcessNum;i++){
        enqueueProcess(&readyQ,sortingQ[i]);
    }
    sortingProcessNum=0;
}

/*Updating the processes in the Device IO/waiting and ensure they are stepping through time.*/
void updateDeviceQ(void){
    int size = DeviceQ.size;

    for(int i=0;i<size;i++){
        process *p=DeviceQ.front->data;
        dequeueProcess(&DeviceQ)
        p->bursts[p->currentBurst].step++;
        if(p->bursts[p->currentBurst].step== p->bursts[p->currentBurst].length){
            sortingQ[sortingProcessNum]=p;
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

    for(int i=0;i<size;i++){
    process *p=readyQ.front->data;
    dequeueProcess(&readyQ)
    p->waitingTime++;
    enqueueProcess(&readyQ,p);

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
    process processes[MAX_PROCESSES+1];   
    int status;

    while ((status=readProcess(&processes[numberOfProcesses]))!=0)  {
         if(status==1)  numberOfProcesses ++;
         printf("Process %d added to array\n",processes[numberOfProcesses-1].pid);
    } 
    qsort(processes, numberOfProcesses, sizeof(process), compareByArrivalandPID);

    /*Initialize all the Processes*/
    initializeGlobals();
    printf("Initialized Globals Succesfully\n");


    while(systemrunning){

        /*check if there are processes arriving at this current time and add them. They have already been sorted by PID if the arrivalTime is the same*/
        while(processes[nextProcess].arrivalTime == simulationtime){
            enqueueProcess(&readyQ,&processes[nextProcess]);  
            nextProcess++;          
        }

        sorter();//add any waiting processes that needed to be sorted to the readyQ
        cpuOperation();//Conduct the CPU Operations as necessary
        updateDeviceQ();//Update the steps in device Q and move them to sorting if done.
        updateReadyQ();//Update waiting time for processes

        if(nextProcess>=numberOfProcesses && readyQ.front=NULL && DeviceQ.front=NULL){
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
        totalTurnAroundTime += (processes[i].endTime-processes[i].arrivalTimeTime);
        if(processes[i].endTime==simulationtime){
            lastpid=processes[i].pid;
        }
    }
    double avgWait = totalWaitTime/ (double) numberOfProcesses;
    double avgTurnAround = totalTurnAroundTime/ (double) numberOfProcesses;
    double avgCPU=  (cpuActiveTime/simulationtime) *100.00



    printf("Average waiting time:\t%f milliseconds\n
            Average turnaround time:\t%f milliseconds\n",avgWait,avgTurnAround);
    printf("Time all processes finished:\t%d milliseconds\n",simulationtime);
    printf("Average CPU Utilization:\t%f%%\n
            Number of context switches:\t%d\n",avgCPU,numContextSwitch);
    printf("PID of the last process to finish:\t%d\n",lastpid);

}