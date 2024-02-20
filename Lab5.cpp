// Michael Fleagle
// CS470 Lab 5

#include <iostream>
#include <pthread.h>
#include <fstream>
#include <queue>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <bits/stdc++.h>

// namespace declaration
using namespace std;


// create structure for holding process data
#pragma pack(1)
typedef struct process_
{
	// create packed struct based on how the PCB data is given in the Lab5 Documentation
	// each datatype is assigned based on the size needed for each attribute
	char priority;
	char processName[32];
	int processID;
	char activityStatus;
	int CPUBurstTime;
	int baseRegister;
	long int limitRegister;
	char processType;
	int numberOfFiles;
	
} process;


// create global memory to be shared by processors
vector<process> * processQueues;

// vector to manually join
vector<int> runningProcesses;

// mutex lock vairables
pthread_mutex_t queueLock;

// global flag for telling the threads if load balancing is happening
bool loadBalancing = false;

// global flag for telling the threads when everything is complete
bool allProcessesComplete = false;

// thread method to handle FCFS scheduling
void * FCFS(void * param)
{
	int processorNumber = (int)(long)param;
	
	while(allProcessesComplete == false)
	{
		// create a process to hold the current process which is being run on
		process * currentProcess = NULL;
		
		while(loadBalancing == false) //&& processQueues[processorNumber].size() > 0)
		{
			// if the current process is 'empty' then get the next process from the queue
			if(currentProcess == NULL && processQueues[processorNumber].size() > 0) 
			{
				// malloc the size of process for the current process
				
				// set the current process to the next process in the queue
				currentProcess = &processQueues[processorNumber].at(processQueues[processorNumber].size() - 1);
				
				cout << "[FCFS]Processor " << processorNumber << " : Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;			
				cout.flush();
				
				processQueues[processorNumber].pop_back();
			}
			
			if(currentProcess && (currentProcess->CPUBurstTime > 0))
			{
				currentProcess->CPUBurstTime -= 2;
				
				cout << "[FCFS]Processor " << processorNumber << " : Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;
				cout.flush();
				
				sleep(0.2);
			}
			else
			{
				if(currentProcess)
				{
					// tell the user that the process completed
					cout << "[FCFS]Processor " << processorNumber << " has completed process " << currentProcess->processID << endl;
					cout.flush();
					
					// process has completed
					currentProcess = NULL;
				}
			}
			
			// currentProcess = NULL;
			
			if(currentProcess == NULL && processQueues[processorNumber].size() == 0)
			{
				break;
			}
		}
	}
	
	cout << "[FCFS]Processor " << processorNumber << " is ending" << endl;
	cout.flush();
	
	pthread_mutex_lock(&queueLock);
	
	runningProcesses.pop_back();
	
	pthread_mutex_unlock(&queueLock);
	
	return (void *) 0;
}


// thread method to handle RR scheduling
void * RR(void * param)
{
	int processorNumber = (int)(long)param;
	
	while(allProcessesComplete == false)
	{
		process * currentProcess = NULL;
		
		while(loadBalancing == false) // && processQueues[processorNumber].size() > 0)
		{
			// if the current process is 'empty' then get the next process from the queue
			if(currentProcess == NULL && processQueues[processorNumber].size() > 0) 
			{	
				// set the current process to the next process in the queue
				currentProcess = &processQueues[processorNumber].at(processQueues[processorNumber].size() - 1);
				
				cout << "[RR]Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;			
				cout.flush();
				
				processQueues[processorNumber].pop_back();
			}
			
			if(currentProcess != NULL)
			{
				currentProcess->CPUBurstTime -= 2;
				
				cout << "[RR]Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;
				cout.flush();
				
				sleep(0.2);
				
				if(currentProcess->CPUBurstTime > 0)
				{
					// push the currentProcess back into the queue
					processQueues[processorNumber].insert(processQueues[processorNumber].begin(), *currentProcess);
					
					cout << "[RR]Switching process" << endl;
					cout.flush();
					
				}
				else
				{
					// tell the user that the process completed
					cout << "[RR]Processor " << processorNumber << " has completed process " << currentProcess->processID << endl;
					cout.flush();
					
				}
				
				currentProcess = NULL;
			}
			
			if(currentProcess == NULL && processQueues[processorNumber].size() == 0)
			{
				break;
			}
		}
	} 
	
	cout << "[RR]Processor " << processorNumber << " is ending" << endl;
	cout.flush();
	
	pthread_mutex_lock(&queueLock);
	
	runningProcesses.pop_back();
	
	pthread_mutex_unlock(&queueLock);
	
	return (void *) 0;
}


// sjf compare method to help with the vector sorting
bool sjfCompare(process p1, process p2)
{
	return (p1.CPUBurstTime > p2.CPUBurstTime);
}


// thread method to handle SJF scheduling
void * SJF(void * param)
{
	int processorNumber = (int)(long)param;
	
	// do a vector sort of the processes by their CPUBurstTime
	sort(processQueues[processorNumber].begin(), processQueues[processorNumber].end(), sjfCompare);
	
	while(allProcessesComplete == false)
	{
		// create a process to hold the current process which is being run on
		process * currentProcess = NULL;
		
		while(loadBalancing == false) // && processQueues[processorNumber].size() > 0)
		{
			// if the current process is 'empty' then get the next process from the queue
			if(currentProcess == NULL && processQueues[processorNumber].size() > 0) 
			{
				// malloc the size of process for the current process
				// currentProcess = (process *)malloc(sizeof(process));
				
				// set the current process to the next process in the queue
				currentProcess = &processQueues[processorNumber].at(processQueues[processorNumber].size() - 1);
				
				cout << "[SJF]Process is starting on process: " << currentProcess->processID << " with CPU Burst Time of: " << currentProcess->CPUBurstTime << endl;			
				cout.flush();
				
				processQueues[processorNumber].pop_back();
			}
			
			if(currentProcess != NULL)
			{	
				if(currentProcess->CPUBurstTime > 0)
				{
					currentProcess->CPUBurstTime -= 2;
					
					cout << "[SJF]Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;
					
					cout.flush();
					
					sleep(0.2);
				}
				else
				{
					if(currentProcess)
					{
						// tell the user that the process completed
						cout << "[SJF]Processor " << processorNumber << " has completed process " << currentProcess->processID << endl;
						cout.flush();
						
						currentProcess = NULL;
					}
				}
			}
			
			if(currentProcess == NULL && processQueues[processorNumber].size() == 0)
			{
				break;
			}
			
		}
		
		if(loadBalancing == true && processQueues[processorNumber].size() > 2)
		{
			cout << "[SJF]Processor " << processorNumber << " resorting due to load balance" << endl;
			cout.flush();
			
			// do a vector sort of the processes by their CPUBurstTime
			sort(processQueues[processorNumber].begin(), processQueues[processorNumber].end(), sjfCompare);
		}
		
		// currentProcess = NULL; 
	}
	cout << "[SJF]Processor " << processorNumber << " is ending" << endl;
	cout.flush();
	
	pthread_mutex_lock(&queueLock);
	
	runningProcesses.pop_back();
	
	pthread_mutex_unlock(&queueLock);
	
	return (void *) 0;
}


// pr sort. sort processes by priority
bool prCompare(process p1, process p2)
{
	return (p1.priority < p2.priority);
}


// aging method. adds 1 to the priority of each process still in the queue
void aging(int processorNumber)
{
	for(int i = 0; i < processQueues[processorNumber].size(); i++)
	{
		processQueues[processorNumber].at(i).priority -= 1;
	}
	
	cout << "[PS]" << processorNumber << " processor has aged its process queue" << endl;
	cout.flush();
}


// thread method to handle PS scheduling
void * PS(void * param)
{
	int processorNumber = (int)(long)param;
	
	// double variable to hold the amount of time that has passed (for aging)
	double time = 0.0;
	
	// sort the processes by their priority
	sort(processQueues[processorNumber].begin(), processQueues[processorNumber].end(), prCompare);
	
	while(allProcessesComplete == false)
	{
		// create a process to hold the current process which is being run on
		process * currentProcess = NULL;
		
		while(loadBalancing == false) //&& processQueues[processorNumber].size() > 0)
		{
			if(time == 2.0)
			{
				// call the aging method
				aging(processorNumber);
				
				time = 0.0;
			}
			else
			{
				// if the current process is 'empty' then get the next process from the queue
				if(currentProcess == NULL && processQueues[processorNumber].size() > 0) 
				{
					// malloc the size of process for the current process
					// currentProcess = (process *)malloc(sizeof(process));
					
					// set the current process to the next process in the queue
					currentProcess = &processQueues[processorNumber].at(processQueues[processorNumber].size() - 1);
					
					cout << "[PS]Process " << currentProcess->processID << " is started with a priority of: " << currentProcess->priority << endl;			
					cout.flush();
					
					processQueues[processorNumber].pop_back();
				}
				if(currentProcess != NULL)
				{
					if(currentProcess->CPUBurstTime > 0)
					{
						currentProcess->CPUBurstTime -= 2;
						
						cout << "[PS]Process " << currentProcess->processID << " has a cpu burst remaining of " << currentProcess->CPUBurstTime << endl;
						
						cout.flush();
						
						sleep(0.2);
						
						time += 0.2;
					}
					else
					{
						// tell the user that the process completed
						cout << "[PS]Processor " << processorNumber << " has completed process " << currentProcess->processID << endl;
						cout.flush();
						
						// process has completed
						currentProcess = NULL;
					}
				}
				
				if(currentProcess == NULL && processQueues[processorNumber].size() == 0)
				{
					break;
				}
			}
		}
		
		if(loadBalancing == true && processQueues[processorNumber].size() > 1)
		{
			cout << "[PS]Processor " << processorNumber << " resorting due to load balance" << endl;
			cout.flush();
			
			sort(processQueues[processorNumber].begin(), processQueues[processorNumber].end(), prCompare);
		}
		
		// currentProcess = NULL;
	}
	cout << "[PS]Processor " << processorNumber << " is ending" << endl;
	cout.flush();
	
	pthread_mutex_lock(&queueLock);
	
	runningProcesses.pop_back();
	
	pthread_mutex_unlock(&queueLock);
	
	return (void *) 0;
}


// method to load balance the queues if told to do so by the loadBalance thread
int queueBalancing(int numProcessors)
{
	cout << "Entering queue balance" << endl;
	cout.flush();
	
	// load balance the queues
	// lock the mutex
	pthread_mutex_lock(&queueLock);
	
	// tell the threads that load balancing is happening
	loadBalancing = true;
	
	// do proper load balancing
	// find the sum of all the sizes of current queues
	int sum = 0;
	
	for(int i = 0; i < numProcessors; i++)
	{
		sum += processQueues[i].size();
	}
	
	// if the sum is 0, end the processors
	if(sum == 0)
	{
		cout << "[Load Balance]All queues are empty\n\n" << endl;
		cout.flush();
		
		loadBalancing = false;
		
		// unlock the mutex
		pthread_mutex_unlock(&queueLock);
		
		allProcessesComplete = true;
		
		return 0;
	}
	
	if(sum <= numProcessors)
	{
		cout << "[Load Balance]Load balance cancelled due to too few processes" << endl;
		cout.flush();
		
		loadBalancing = false;
		
		// unlock the mutex
		pthread_mutex_unlock(&queueLock);
		
		return 3;
	}
	
	// tell the threads that load balancing is happening
	loadBalancing = true;
	
	// determine how many processes should go to each queue
	double numProcessesPerQueue = (sum / numProcessors);
	
	cout << "[Load Balance]There are " << sum << " total processes that need to be redistributed between " << numProcessors << " processors. Each processors should get " << numProcessesPerQueue << " processes each." << endl;
	cout.flush();
	
	// print the queues before redistribution
	cout << "\nThe queues before redistribution look like: " << endl;
	
	for (int i = 0; i < numProcessors; i++)
	{
		cout << "\nProcessor " << i << " queue contains: " << endl; 
		
		for(int j = 0; j < processQueues[i].size(); j++)
		{
			cout << "\tProcess: " << processQueues[i].at(j).processID << endl;
		}
	}
	cout << "\n";
	cout.flush();
	
	// create a temporary vector of processes to hold processes while they are redistributed
	vector<process> toRedistribute;
	
	for(int i = 0; i < numProcessors; i++)
	{
		while(processQueues[i].size() > numProcessesPerQueue)
		{
			toRedistribute.push_back(processQueues[i].at(processQueues[i].size() - 1));
			
			cout << "[Load Balance]Process " << processQueues[i].at(processQueues[i].size() - 1).processID << " removed from processor " << i << " queue." << endl;
			cout.flush();
			
			processQueues[i].pop_back();
		}
	}
	
	// add processes to queues which are less than the redistrobution size
	for(int i = 0; i < numProcessors; i++)
	{
		while(processQueues[i].size() <= numProcessesPerQueue && toRedistribute.size() > 0)
		{
			processQueues[i].insert(processQueues[i].begin(), toRedistribute.at(toRedistribute.size() - 1));
			
			cout << "[Load Balance]Process " << toRedistribute.at(toRedistribute.size() - 1).processID << " added to processor " << i << " queue." << endl;
			cout.flush();
			
			toRedistribute.pop_back();
		}
	}
	
	// print the queues after redistribution
	cout << "\nThe queues after redistribution look like: " << endl;
	
	for (int i = 0; i < numProcessors; i++)
	{
		cout << "\nProcessor " << i << " queue contains: " << endl; 
		
		for(int j = 0; j < processQueues[i].size(); j++)
		{
			cout << "\tProcess: " << processQueues[i].at(j).processID << endl;
		}
	}
	cout << "\n";
	cout.flush();
	
	// unlock the mutex
	pthread_mutex_unlock(&queueLock);
	
	cout << "[Load Balance]Load balancing complete. Processors resuming\n\n" << endl;
	cout.flush();
	
	// tell the threads that load balancing is complete
	loadBalancing = false;
	
	return 1;
}


// thread method to handle load balancing
void * loadBalance(void * param)
{	
	// get the number of processors from the param
	int numProcessors = (int)(long)param;
	
	bool endBalanceCheck = false;
	
	while(endBalanceCheck == false)
	{
		// check the sizes of each processors queue in processQueues
		for(int i = 0; i < numProcessors; i++)
		{
			if(processQueues[i].size() == 0 && allProcessesComplete == false)
			{
				// tell the user what is happening
				cout << "\n\n[Load Balance]Starting load balancing due to processor " << i << " completing its jobs" << endl; 
				cout.flush();
				
				// call the function to actually handle load balancing
				int balanceReturn = queueBalancing(numProcessors);
				
				if(balanceReturn == 0)
				{
					sleep(0.5);
					endBalanceCheck = true;
				}
				else if(balanceReturn == 3)
				{
					cout << "[Load Balance]Sleeping to allow processors to finish the few processes they have left\n\n" << endl;
					cout.flush();
					
					// sleep the checking function to give the processors some time to finish their remaining processes
					sleep(0.5);
					
				}
				
				break;
			}
		}
		
	}
	
	pthread_mutex_lock(&queueLock);
	
	runningProcesses.pop_back();
	
	pthread_mutex_unlock(&queueLock);
	
	return (void *) 0;
	
}


// method to set the initial process balance as requested
int initialBalance(int argc, char * argv[])
{
	// determine how many process there are
	int numProcesses = 0;
	
	// open the passed file for reading
	vector<process> allPCB;
	
	int pcbIterator = 0;
	
	FILE * pFile = fopen(argv[1], "rb");
	
	if(pFile == NULL)
	{
		cout << "File " << argv[1] << " was not found. Please make sure that the file exists and that the name is correct." << endl;
		cout.flush();
		return -1;
	}
	
	// read from the file into a vector to hold all the PCBs
	while(!feof(pFile))
	{
		process tempProc;
		
		allPCB.push_back(tempProc);
		
		// read from the file each byte size based on the size needed 
		fread(&allPCB.at(pcbIterator).priority, sizeof(char), 1, pFile);
		fread(&allPCB.at(pcbIterator).processName, (sizeof(char) * 32), 1, pFile);
		fread(&allPCB.at(pcbIterator).processID, sizeof(int), 1, pFile);
		fread(&allPCB.at(pcbIterator).activityStatus, sizeof(char), 1, pFile);
		fread(&allPCB.at(pcbIterator).CPUBurstTime, sizeof(int), 1, pFile);
		fread(&allPCB.at(pcbIterator).baseRegister, sizeof(int), 1, pFile);
		fread(&allPCB.at(pcbIterator).limitRegister, sizeof(long int), 1, pFile);
		fread(&allPCB.at(pcbIterator).processType, sizeof(char), 1, pFile);
		fread(&allPCB.at(pcbIterator).numberOfFiles, sizeof(int), 1, pFile);
		
		pcbIterator++;
	}
	
	// pop the last process because it reads one too many
	allPCB.pop_back();
	
	// close the file
	fclose(pFile);
	
	// test print the pcb
	// printPCB(allPCB);
	
	numProcesses = allPCB.size();
	
	// determine how many each process should get
	vector<int> numProcessesPerProcessor;
	
	for(int i = 3; i < argc; i+=2)
	{
		numProcessesPerProcessor.push_back((strtod(argv[i], 0) * numProcesses));
	}
	
	// put the processes into the queues
	// NOTE: queues are represented as vectors
	for(int i = 0; i < numProcessesPerProcessor.size(); i++)
	{
		while(processQueues[i].size() < numProcessesPerProcessor.at(i))
		{
			// since we are using vectors to represent the queues, we read the processes backwards to preserver their order (we will be reading processes from the back)
			processQueues[i].push_back(allPCB.back());
			
			// remove the last PCB added
			allPCB.pop_back();
		}
	}
	
	// return 0 if there are no issues
	return 0;
}


// method to determine if the input arguments are valid
int argCheck(int argc, char * argv[])
{	
	// there must be at least 3 argc
	if(argc < 4)
	{
		cout << "Error, too few arguments input. \n\tCorrect usage is: processFile process1Type process1Ratio ... [processNType] [processNRatio]" << endl;
		
		return 1;
	}
	
	// there should be (nProcesses * 2) + 1 inputs
	if((argc % 2) != 0)
	{
		cout << "Error, the number of inputs is not correct. \n\tCorrect usage is: processFile process1Type process1Ratio ... [processNType] [processNRatio]" << endl;
		
		return 2;
	}
	
	// make sure that all of the ratios add up to 1
	double inputRatioValue = 0.0;
	
	for(int i = 3; i < argc; i+=2)
	{
		inputRatioValue += stod(argv[i]);
	}
	
	if (inputRatioValue != 1.0)
	{
		cout << "Error, the ratios do not add up to 1.0. Please ensure that the total of all process ratios is 1.0" << endl;
		
		return 3;
	}
	
	// check that all of the process types are valid
	// priority sched = 4, short job first = 3, round robin = 2, FCFS = 1
	for(int i = 2; i < argc; i+=2)
	{
		if (atoi(argv[i]) != 1 && atoi(argv[i]) != 2 && atoi(argv[i]) != 3 && atoi(argv[i]) != 4)
		{
			cout << "Error, processor number " << (i/2) + 1 << " is not a recognized type. The proper types are: \n\tFirst Come First Server (FCFS) = 1\n\tRound Robin = 2\n\tShortest Job First (SJF) = 3\n\tPriority Scheduling = 4" << endl;
			
			return 4;
		}
	}
	
	return 0;
}


// main method
int main(int argc, char* argv[])
{
	// check if the arguments are valid
	int argVal = argCheck(argc, argv);
	
	// if the arguments are not valid, exit the program
	if(argVal != 0)
	{
		return argVal;
	}
	
	int numProcessors = (argc - 2) / 2;
	
	cout << "The size size allocated will be " << sizeof(vector<process>) << endl;
	
	// allocate the necessary memory for the processQueues global variable
	processQueues = (vector<process> *) malloc(numProcessors * sizeof(vector<process>));
	
	// create process queues for each thread using vectors
	for(int i = 0; i < numProcessors; i++)
	{
		vector<process> tempQueue;
		processQueues[i] = tempQueue;
	}
	
	// put processes into queues based on requested balance
	if(initialBalance(argc, argv) == -1)
	{
		return -1;
	}
	
	// determine what type of processor algorithm each thread should use and how many should be made
	vector<pthread_t> threadFCFS;
	vector<pthread_t> threadRR;
	vector<pthread_t> threadSJF;
	vector<pthread_t> threadPS;
	
	// tell the user what is happening
	cout << "Creating processors" << endl;
	cout.flush();
	
	// variable to hold an id for each processor
	int processorID = 0;
	
	// create threads based on the type given
	for(int i = 2; i < argc; i+=2)
	{
		int temp = atoi(argv[i]);
		
		if(temp == 1)
		{
			cout << "Creating FCFS processor" << endl;
			cout.flush();
			
			// create FCFS
			pthread_t tempFCFS; 
			
			threadFCFS.push_back(tempFCFS);
			
			pthread_create(&threadFCFS.back(), NULL, FCFS, (void *)(long)processorID);
			
			pthread_mutex_lock(&queueLock);
			runningProcesses.push_back(processorID);
			pthread_mutex_unlock(&queueLock);
			
			
			processorID++;
		}
		else if(temp == 2)
		{
			cout << "Creating RR processor" << endl;
			cout.flush();
			
			// create RR
			pthread_t tempRR; 
			
			threadRR.push_back(tempRR);
			
			pthread_create(&threadRR.back(), NULL, RR, (void *)(long)processorID);
			
			pthread_mutex_lock(&queueLock);
			runningProcesses.push_back(processorID);
			pthread_mutex_unlock(&queueLock);
			
			processorID++;
		}
		else if(temp == 3)
		{
			cout << "Creating SJF processor" << endl;
			cout.flush();
			
			// create SJF
			pthread_t tempSJF; 
			
			threadSJF.push_back(tempSJF);
			
			pthread_create(&threadSJF.back(), NULL, SJF, (void *)(long)processorID);
			
			pthread_mutex_lock(&queueLock);
			runningProcesses.push_back(processorID);
			pthread_mutex_unlock(&queueLock);
			
			processorID++;
		}
		else if(temp == 4)
		{
			cout << "Creating PS processor" << endl;
			cout.flush();
			
			// create PS
			pthread_t tempPS; 
			
			threadPS.push_back(tempPS);
			
			pthread_create(&threadPS.back(), NULL, PS, (void *)(long)processorID);
			
			pthread_mutex_lock(&queueLock);
			runningProcesses.push_back(processorID);
			pthread_mutex_unlock(&queueLock);
			
			processorID++;
		}
	}
	
	// create master thread to handle load balancing
	pthread_t loadBalanceThread;
	
	pthread_create(&loadBalanceThread, NULL, loadBalance, (void *)(long)numProcessors);
	
	pthread_mutex_lock(&queueLock);
	runningProcesses.push_back(processorID);
	pthread_mutex_unlock(&queueLock);
	
	// join all threads manually
	while(runningProcesses.size() > 0)
	{
		// manual joining waiting for threads to finish
	}
	
	// check that all processes were completed
	for(int i = 0; i < numProcessors; i++)
	{
		if(processQueues[i].size() > 0)
		{
			cout << "Error, not all processes completed" << endl;
		}
	}
	
	cout << "\nAll processes have completed" << endl;

	free(processQueues);
	
	return 0;
}