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
process cpu[NUMofCores];

int numberOfProcesses;  
int cpuActiveTime;
int simulationtime;
int nextProcess;
int quantumTime;
int numContextSwitch;
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

    systemrunning=true;
}

/*CPU Operations.*/
void cpuOperation(void){

    for(int i=0;i<NUMofCores;i++){
        if(cpu[i]==NULL){
            if(readyQ->front != NULL){
                cpu[i]=readyQ.front->data;
                dequeueProcess(readyQ);
                if(cpu.startTime == 0 && cpu.bursts[0].step==0 ){
                    cpu[i].startTime=simulationtime;
                }
                cpu.quantumRemaining=quantumTime;


            }
        }

        if(cpu[i]!= NULL){
            cpu[i].bursts[cpu[i].currentBurst].step++;
            if(cpu[i].bursts[cpu[i].currentBurst].step == cpu[i].bursts[cpu[i].currentBurst].length){
                cpu[i].quantumRemaining=0;
                cpu[i].currentBurst++;
                enqueueProcess(&DeviceQ,cpu[i]);
            }
            cpuActiveTime++;
            if(cpu[i].currentBurst==cpu[i].numberOfBursts){
                cpu[i].endTime=simulationtime;
                
            }
            cpu[i].quantumRemaining--;
            if(cpu[i].quantumRemaining==0){

            }

        }
    }

}

int main(int argc, char* argv[])
{
    /*Check if the correct time quantum arguments are passed through*/
    if(argc <= 1 || argc > 2){
        error_bad_quantum();
    }
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


    while(systemrunning){

        /*check if there are processes arriving at this current time and add them. They have already been sorted by PID if the arrivalTime is the same*/
        while(processes[nextProcess].arrivalTime == simulationtime){
            enqueueProcess(readyQ,processes[nextProcess]);            
        }




    }

}