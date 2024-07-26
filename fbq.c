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

process_queue Queue0;
process_queue Queue1;
process_queue Queue2;
process_queue DeviceQ;
process_queue completedQ;
process *cpu[NUMofCores];
process *sortingQ0[MAX_PROCESSES+1];
process *sortingQ1[MAX_PROCESSES+1];
process *sortingQ2[MAX_PROCESSES+1];
process processes[MAX_PROCESSES+1];   

int numberOfProcesses;  
int cpuActiveTime;
int simulationtime;
int nextProcess;
int quantumTime0;
int quantumTime1;
int numContextSwitch;
int sortingProcessNum0;
int sortingProcessNum1;
int sortingProcessNum2;
bool systemrunning;


/*Initilaize all the Global varialbles */
void initializeGlobals(void){

    initializeProcessQueue(&Queue0);
    initializeProcessQueue(&Queue1);
    initializeProcessQueue(&Queue2);
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

        /*if the CPU isn't busy, pull from the Queue1 then Queue2 if available*/
        if(cpu[i]==NULL){.

            /*Lowest Level Queue*/
            if(Queue0.front != NULL){
                cpu[i]=Queue0.front->data;
                printf("about to deque from Queue0 of size %d\n",Queue0.size);
                //sleep(2);
                dequeueProcess(&Queue0);
                if(cpu[i]->startTime < cpu[i]->arrivalTime ){
                    cpu[i]->startTime=simulationtime;
                    cpu[i]->waitingTime=cpu[i]->startTime-cpu[i]->arrivalTime;
                }
                cpu[i]->quantumRemaining=quantumTime0;
                cpu[i]->currentQueue=0;
            }
            /*Mid Level Queue*/
            if(Queue1.front != NULL){
                cpu[i]=Queue1.front->data;
                printf("about to deque from Queue1 of size %d\n",Queue1.size);
                //sleep(2);
                dequeueProcess(&Queue1);
                if(cpu[i]->startTime < cpu[i]->arrivalTime ){
                    cpu[i]->startTime=simulationtime;
                    cpu[i]->waitingTime=cpu[i]->startTime-cpu[i]->arrivalTime;
                }
                cpu[i]->quantumRemaining=quantumTime1;
                cpu[i]->currentQueue=1;
            }

            /*Highest Level Queue*/
            if(Queue2.front != NULL){
                cpu[i]=Queue2.front->data;
                printf("about to deque from Queue2 of size %d\n",Queue2.size);
                //sleep(2);
                dequeueProcess(&Queue2);
                if(cpu[i]->startTime < cpu[i]->arrivalTime ){
                    cpu[i]->startTime=simulationtime;
                    cpu[i]->waitingTime=cpu[i]->startTime-cpu[i]->arrivalTime;
                }
                cpu[i]->quantumRemaining=0;
                cpu[i]->currentQueue=2;
            }
        }

        /*If the CPU is busy, continue processing*/
        if(cpu[i]!= NULL){
            
            /*Low level Queue operation*/
            if(cpu[i]->currentQueue==0){
                cpuActiveTime++;
                cpu[i]->quantumRemaining--;
                cpu[i]->bursts[cpu[i]->currentBurst].step++;

                /*When the current burst is finished, move to IO if more bursts, end if not*/
                if(cpu[i]->bursts[cpu[i]->currentBurst].step >= cpu[i]->bursts[cpu[i]->currentBurst].length){
                    printf("burst completed on CPU %d for pid %d. Current burst: %d. Bursts left: %d.\n",i,cpu[i]->pid,cpu[i]->currentBurst, cpu[i]->numberOfBursts);
                    cpu[i]->quantumRemaining=0;
                    if(cpu[i]->currentBurst < cpu[i]->numberOfBursts){
                        enqueueProcess(&DeviceQ,cpu[i]);
                        cpu[i]->currentBurst++;
                        cpu[i]=NULL;
                        printf("Moved to DeviceQ from CPU %d\n",i);
                        continue;
                    }
                    else if(cpu[i]->currentBurst>=cpu[i]->numberOfBursts){
                        cpu[i]->endTime=simulationtime;
                        enqueueProcess(&completedQ,cpu[i]);
                        cpu[i]=NULL;
                        //                     printf("WHERE IS MY ISSUE!\n");
                        // exit(1);
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

}

/*Sort processes with the same scheduling criteria*/
void sorter(void){

    /*Q0*/
    qsort(sortingQ0,sortingProcessNum0,sizeof(process*),compareByPID);
    if(sortingProcessNum0>0){
            printf("Current amount of sorting processes %d\n",sortingProcessNum0);
    }

    for(int i=0; i<sortingProcessNum0;i++){
        enqueueProcess(&Queue0,sortingQ0[i]);
        printf("Process %d has been sorted and added to readyQ.  Size is now %d\n",sortingQ0[i]->pid,Queue0.size);
        //sleep(2);
    }
    sortingProcessNum0=0;

    /*Q1*/
    qsort(sortingQ1,sortingProcessNum1,sizeof(process*),compareByPID);
    if(sortingProcessNum1>0){
            printf("Current amount of sorting processes %d\n",sortingProcessNum1);
    }

    for(int i=0; i<sortingProcessNum1;i++){
        enqueueProcess(&Queue1,sortingQ1[i]);
        printf("Process %d has been sorted and added to readyQ.  Size is now %d\n",sortingQ1[i]->pid,Queue1.size);
        //sleep(2);
    }
    sortingProcessNum1=0;


    /*Q2*/
    qsort(sortingQ2,sortingProcessNum2,sizeof(process*),compareByPID);
    if(sortingProcessNum2>0){
            printf("Current amount of sorting processes %d\n",sortingProcessNum2);
    }

    for(int i=0; i<sortingProcessNum2;i++){
        enqueueProcess(&Queue2,sortingQ2[i]);
        printf("Process %d has been sorted and added to readyQ.  Size is now %d\n",sortingQ2[i]->pid,Queue2.size);
        //sleep(2);
    }
    sortingProcessNum2=0;
}

/*Updating the processes in the Device IO/waiting and ensure they are stepping through time.*/
void updateDeviceQ(void){
    int size = DeviceQ.size;

    for(int i=0;i<size;i++){
        process *p=DeviceQ.front->data;
        dequeueProcess(&DeviceQ);
        p->bursts[p->currentBurst].step++;
        printf("we are step %d of %d for pid %d\n",p->bursts[p->currentBurst].step,p->bursts[p->currentBurst].length,p->pid);
        //sleep(.5);
        if(p->bursts[p->currentBurst].step >= p->bursts[p->currentBurst].length){
            sortingQ0[sortingProcessNum0]=p;
            sortingProcessNum0++;
            printf("This is the new number of processes to sort for Q0:%d\n",sortingProcessNum0);
            p->currentBurst++;
            printf("The next CPU burst for process %d is %d\n",p->pid,p->currentBurst);
            //sleep(2);
            continue;
        }
        else{
            enqueueProcess(&DeviceQ,p);
        }

    }
}

void updateQ0(void){
    int size = Queue0.size;
    printf("Updating readyQ of size %d\n",size);
    for(int i=0;i<size;i++){
    process *p=Queue0.front->data;
    dequeueProcess(&Queue0);
    p->waitingTime++;
    enqueueProcess(&Queue0,p);
    printf("Process %d updated\n",p->pid);

    }
}

void updateQ1(void){
    int size = Queue1.size;
    printf("Updating readyQ of size %d\n",size);
    for(int i=0;i<size;i++){
    process *p=Queue1.front->data;
    dequeueProcess(&Queue1);
    p->waitingTime++;
    enqueueProcess(&Queue1,p);
    printf("Process %d updated\n",p->pid);

    }
}

void updateQ2(void){
    int size = Queue2.size;
    printf("Updating readyQ of size %d\n",size);
    for(int i=0;i<size;i++){
    process *p=Queue2.front->data;
    dequeueProcess(&Queue2);
    p->waitingTime++;
    enqueueProcess(&Queue2,p);
    printf("Process %d updated\n",p->pid);

    }
}

int isCPUrunning(void){

    for(int i=0;i<NUMofCores;i++){
        if(cpu[i]!=NULL){
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    /*Check if the correct time quantum arguments are passed through*/
    if(argc <= 1 || argc > 2){
        error_bad_quantum();
    }

    /*Initialize quantum time to provided parameter*/
    quantumTime0 = atoi(argv[1]);
    quantumTime1 = atoi(argv[2]);

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
        //Update waiting time for processes
        updateQ0();
        updateQ1();
        updateQ2();


       printf("nextProcess: %d, numberOfProcesses: %d, readyQ.front: %p, readyQ.size: %d, DeviceQ.front: %p DeviceQ.size: %d, completedQ.size %d\n",
       nextProcess, numberOfProcesses, (void *)readyQ.front, readyQ.size, (void *)DeviceQ.front, DeviceQ.size, completedQ.size);

       if( nextProcess >= numberOfProcesses && readyQ.size ==0 && DeviceQ.size==0 && isCPUrunning()){
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
        
    }
    double avgWait = totalWaitTime/ (double) numberOfProcesses;
    double avgTurnAround = totalTurnAroundTime/ (double) numberOfProcesses;
    double avgCPU=  100.00*(cpuActiveTime/(double) simulationtime);
    lastpid=completedQ.back->data->pid;
    printf("This is your cpuActiveTime %d\n",cpuActiveTime);



    printf("Average waiting time:\t%.2f milliseconds\n"
            "Average turnaround time:\t%.2f milliseconds\n",avgWait,avgTurnAround);
    printf("Time all processes finished:\t%d milliseconds\n",simulationtime);
    printf("Average CPU Utilization:\t%.2f%%\n"
            "Number of context switches:\t%d\n",avgCPU,numContextSwitch);
    printf("PID of the last process to finish:\t%d\n",lastpid);

    return 0;

}