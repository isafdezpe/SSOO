#include "OperatingSystem.h"
#include "OperatingSystemBase.h"
#include "MMU.h"
#include "Processor.h"
#include "Buses.h"
#include "Heap.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Functions prototypes
void OperatingSystem_PrepareDaemons(int);
void OperatingSystem_PCBInitialization(int, int, int, int, int, int);
void OperatingSystem_MoveToTheREADYState(int);
void OperatingSystem_Dispatch(int);
void OperatingSystem_RestoreContext(int);
void OperatingSystem_SaveContext(int);
void OperatingSystem_TerminateProcess();
int OperatingSystem_LongTermScheduler();
void OperatingSystem_PreemptRunningProcess();
int OperatingSystem_CreateProcess(int, int);
int OperatingSystem_ObtainMainMemory(int, int);
int OperatingSystem_ShortTermScheduler();
int OperatingSystem_ExtractFromReadyToRun(int);
void OperatingSystem_HandleException();
void OperatingSystem_HandleSystemCall();
void OperatingSystem_PrintReadyToRunQueue();
void OperatingSystem_HandleClockInterrupt();
void OperatingSystem_MoveToTheBlockedState(int);
int OperatingSystem_ExtractFromBlocked();
void OperatingSystem_CheckIfIsNecessaryToChangeProcess();

// The process table
PCB processTable[PROCESSTABLEMAXSIZE];

// Address base for OS code in this version
int OS_address_base = PROCESSTABLEMAXSIZE * MAINMEMORYSECTIONSIZE;

// Identifier of the current executing process
int executingProcessID=NOPROCESS;

// Identifier of the System Idle Process
int sipID;

// Initial PID for assignation
int initialPID=PROCESSTABLEMAXSIZE - 1;

// Begin indes for daemons in programList
int baseDaemonsInProgramList; 

// Number of clock interrupts occurred
int numberOfClockInterrupts = 0;

// Array that contains the identifiers of the READY processes
heapItem readyToRunQueue [NUMBEROFQUEUES][PROCESSTABLEMAXSIZE];
int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0,0};

char * queueNames [NUMBEROFQUEUES]={"USER","DAEMONS"}; 

// Heap with blocked processes sort by when to wakeup
heapItem sleepingProcessesQueue[PROCESSTABLEMAXSIZE];
int numberOfSleepingProcesses=0; 

// Variable containing the number of not terminated user processes
int numberOfNotTerminatedUserProcesses=0;

// Names of the process states
char * statesNames [5]={"NEW","READY","EXECUTING","BLOCKED","EXIT"};

// Initial set of tasks of the OS
void OperatingSystem_Initialize(int daemonsIndex) {
	
	int i, selectedProcess, createdProcess;
	FILE *programFile; // For load Operating System Code

	// Obtain the memory requirements of the program
	int processSize=OperatingSystem_ObtainProgramSize(&programFile, "OperatingSystemCode");

	// Load Operating System Code
	OperatingSystem_LoadProgram(programFile, OS_address_base, processSize);
	
	// Process table initialization (all entries are free)
	for (i=0; i<PROCESSTABLEMAXSIZE;i++){
		processTable[i].busy=0;
	}
	// Initialization of the interrupt vector table of the processor
	Processor_InitializeInterruptVectorTable(OS_address_base+2);
		
	// Include in program list  all system daemon processes
	OperatingSystem_PrepareDaemons(daemonsIndex);
	
	// Create all user processes from the information given in the command line
	createdProcess = OperatingSystem_LongTermScheduler();

	if (createdProcess < 1) {
		OperatingSystem_ReadyToShutdown();
	}
	
	if (strcmp(programList[processTable[sipID].programListIndex]->executableName,"SystemIdleProcess")) {
		// Show red message "FATAL ERROR: Missing SIP program!\n"
		ComputerSystem_DebugMessage(99,SHUTDOWN,"FATAL ERROR: Missing SIP program!\n");
		exit(1);		
	}

	// At least, one user process has been created
	// Select the first process that is going to use the processor
	selectedProcess=OperatingSystem_ShortTermScheduler();

	// Assign the processor to the selected process
	OperatingSystem_Dispatch(selectedProcess);

	// Initial operation for Operating System
	Processor_SetPC(OS_address_base);
}

// Daemon processes are system processes, that is, they work together with the OS.
// The System Idle Process uses the CPU whenever a user process is able to use it
void OperatingSystem_PrepareDaemons(int programListDaemonsBase) {
  
	// Include a entry for SystemIdleProcess at 0 position
	programList[0]=(PROGRAMS_DATA *) malloc(sizeof(PROGRAMS_DATA));

	programList[0]->executableName="SystemIdleProcess";
	programList[0]->arrivalTime=0;
	programList[0]->type=DAEMONPROGRAM; // daemon program

	sipID=initialPID%PROCESSTABLEMAXSIZE; // first PID for sipID

	// Prepare aditionals daemons here
	// index for aditionals daemons program in programList
	baseDaemonsInProgramList=programListDaemonsBase;

}


// The LTS is responsible of the admission of new processes in the system.
// Initially, it creates a process from each program specified in the 
// 			command lineand daemons programs
int OperatingSystem_LongTermScheduler() {
  
	int PID, i,
		numberOfSuccessfullyCreatedProcesses=0;
	
	for (i=0; programList[i]!=NULL && i<PROGRAMSMAXNUMBER ; i++) {
		if (programList[i]->type == DAEMONPROGRAM)
			PID=OperatingSystem_CreateProcess(i, DAEMONSQUEUE);
		else
			PID=OperatingSystem_CreateProcess(i, USERPROCESSQUEUE);

		switch (PID)
		{
		case NOFREEENTRY:
			ComputerSystem_DebugMessage(103, ERROR, programList[i] -> executableName);
			break;
		case PROGRAMDOESNOTEXIST:
			ComputerSystem_DebugMessage(104, ERROR, programList[i] -> executableName, "it does not exist");
			break;
		case PROGRAMNOTVALID:
			ComputerSystem_DebugMessage(104, ERROR, programList[i] -> executableName, "invalid priority or size");
			break;
		case TOOBIGPROCESS:
			ComputerSystem_DebugMessage(105, ERROR, programList[i] -> executableName);
		default:
			numberOfSuccessfullyCreatedProcesses++;
			if (programList[i]->type==USERPROGRAM) 
				numberOfNotTerminatedUserProcesses++;
			// Move process to the ready state
			OperatingSystem_MoveToTheREADYState(PID);
			OperatingSystem_PrintStatus();
			break;
		}
		
	}

	// Return the number of succesfully created processes
	return numberOfSuccessfullyCreatedProcesses;
}


// This function creates a process from an executable program
int OperatingSystem_CreateProcess(int indexOfExecutableProgram, int queueId) {
  
	int PID;
	int processSize;
	int loadingPhysicalAddress;
	int priority;
	int program;
	FILE *programFile;
	PROGRAMS_DATA *executableProgram=programList[indexOfExecutableProgram];

	// Obtain a process ID
	PID=OperatingSystem_ObtainAnEntryInTheProcessTable();

	// Check for free entries at proccess table
	if (PID == NOFREEENTRY)
		return NOFREEENTRY;

	// Obtain the memory requirements of the program
	processSize=OperatingSystem_ObtainProgramSize(&programFile, executableProgram->executableName);	

	// Check if the program exists or is valid
	if (processSize == PROGRAMDOESNOTEXIST) 
		return PROGRAMDOESNOTEXIST;
	
	if (processSize == PROGRAMNOTVALID) 
		return PROGRAMNOTVALID;
	
	// Obtain the priority for the process
	priority=OperatingSystem_ObtainPriority(programFile);

	// Check if the priority is valid
	if (priority == PROGRAMNOTVALID) {
		return PROGRAMNOTVALID;
	}
	
	// Obtain enough memory space
 	loadingPhysicalAddress=OperatingSystem_ObtainMainMemory(processSize, PID);

	// Check if the program size is valid
	if (loadingPhysicalAddress == TOOBIGPROCESS)
		return TOOBIGPROCESS;

	// Load program in the allocated memory
	program = OperatingSystem_LoadProgram(programFile, loadingPhysicalAddress, processSize);

	// Check if the number of instructions is valid
	if (program == TOOBIGPROCESS)
		return TOOBIGPROCESS;
	
	// PCB initialization
	OperatingSystem_PCBInitialization(PID, loadingPhysicalAddress, processSize, priority, indexOfExecutableProgram, queueId);
	
	// Show message "Process [PID] created from program [executableName]\n"
	OperatingSystem_ShowTime(INIT);
	ComputerSystem_DebugMessage(70,INIT,PID,executableProgram->executableName);
	
	return PID;
}


// Main memory is assigned in chunks. All chunks are the same size. A process
// always obtains the chunk whose position in memory is equal to the processor identifier
int OperatingSystem_ObtainMainMemory(int processSize, int PID) {

 	if (processSize>MAINMEMORYSECTIONSIZE)
		return TOOBIGPROCESS;
	
 	return PID*MAINMEMORYSECTIONSIZE;
}


// Assign initial values to all fields inside the PCB
void OperatingSystem_PCBInitialization(int PID, int initialPhysicalAddress, int processSize, int priority, int processPLIndex, int queueId) {

	processTable[PID].busy=1;
	processTable[PID].initialPhysicalAddress=initialPhysicalAddress;
	processTable[PID].processSize=processSize;
	processTable[PID].state=NEW;
	processTable[PID].priority=priority;
	processTable[PID].programListIndex=processPLIndex;
	OperatingSystem_ShowTime(SYSPROC);
	ComputerSystem_DebugMessage(111, SYSPROC, PID, programList[processTable[PID].programListIndex] -> executableName, statesNames[0]);
	// Daemons run in protected mode and MMU use real address
	if (programList[processPLIndex]->type == DAEMONPROGRAM) {
		processTable[PID].copyOfPCRegister=initialPhysicalAddress;
		processTable[PID].copyOfPSWRegister= ((unsigned int) 1) << EXECUTION_MODE_BIT;
		processTable[PID].copyOfAccumulatorRegister = 0;
	} 
	else {
		processTable[PID].copyOfPCRegister=0;
		processTable[PID].copyOfPSWRegister=0;
		processTable[PID].copyOfAccumulatorRegister = 0;
	}
	processTable[PID].queueID=queueId;
}


// Move a process to the READY state: it will be inserted, depending on its priority, in
// a queue of identifiers of READY processes
void OperatingSystem_MoveToTheREADYState(int PID) {
	int previousState;

	if (Heap_add(PID, readyToRunQueue[processTable[PID].queueID],QUEUE_PRIORITY ,&numberOfReadyToRunProcesses[processTable[PID].queueID] ,PROCESSTABLEMAXSIZE)>=0) {
		previousState = processTable[PID].state;
		processTable[PID].state=READY;
		OperatingSystem_ShowTime(SYSPROC);
		ComputerSystem_DebugMessage(110, SYSPROC, PID, programList[processTable[PID].programListIndex] -> executableName, statesNames[previousState], statesNames[1]);
	} 

	//OperatingSystem_PrintReadyToRunQueue();
}

//Move a process to the BLOCKED state
void OperatingSystem_MoveToTheBlockedState(int PID) {
	int previousState;
	
	if (Heap_add(PID,sleepingProcessesQueue,QUEUE_WAKEUP,&numberOfSleepingProcesses,PROCESSTABLEMAXSIZE) >= 0) {
		previousState = processTable[PID].state;
		processTable[PID].state = BLOCKED;
		OperatingSystem_ShowTime(SYSPROC);
		ComputerSystem_DebugMessage(110, SYSPROC, PID, programList[processTable[PID].programListIndex] -> executableName, statesNames[previousState], statesNames[3]);

	}
}


// The STS is responsible of deciding which process to execute when specific events occur.
// It uses processes priorities to make the decission. Given that the READY queue is ordered
// depending on processes priority, the STS just selects the process in front of the READY queue
int OperatingSystem_ShortTermScheduler() {
	
	int selectedProcess;
	int i;

	for (i = 0; i < NUMBEROFQUEUES; i++) {
		selectedProcess=OperatingSystem_ExtractFromReadyToRun(i);
		if (selectedProcess != NOPROCESS)
			return selectedProcess;
	}

	return selectedProcess;
}


// Return PID of more priority process in the READY queue
int OperatingSystem_ExtractFromReadyToRun(int queueId) {
  
	int selectedProcess=NOPROCESS;

	selectedProcess=Heap_poll(readyToRunQueue[queueId],QUEUE_PRIORITY ,&numberOfReadyToRunProcesses[queueId]);
	
	// Return most priority process or NOPROCESS if empty queue
	return selectedProcess; 
}

// Return PID of the process with greater whenToWakeUp
int OperatingSystem_ExtractFromBlocked() {
	int selectedProcess = NOPROCESS;

	selectedProcess = Heap_poll(sleepingProcessesQueue,QUEUE_WAKEUP,&numberOfSleepingProcesses);

	return selectedProcess;
}


// Function that assigns the processor to a process
void OperatingSystem_Dispatch(int PID) {
	int previousState;

	// The process identified by PID becomes the current executing process
	executingProcessID=PID;
	previousState = processTable[PID].state;
	// Change the process' state
	processTable[PID].state=EXECUTING;
	OperatingSystem_ShowTime(SYSPROC);
	ComputerSystem_DebugMessage(110, SYSPROC, PID, programList[processTable[PID].programListIndex] -> executableName, statesNames[previousState], statesNames[2]);
	// Modify hardware registers with appropriate values for the process identified by PID
	OperatingSystem_RestoreContext(PID);
}


// Modify hardware registers with appropriate values for the process identified by PID
void OperatingSystem_RestoreContext(int PID) {
  
	// New values for the CPU registers are obtained from the PCB
	Processor_CopyInSystemStack(MAINMEMORYSIZE-1,processTable[PID].copyOfPCRegister);
	Processor_CopyInSystemStack(MAINMEMORYSIZE-2,processTable[PID].copyOfPSWRegister);
	Processor_CopyInSystemStack(MAINMEMORYSIZE-3,processTable[PID].copyOfAccumulatorRegister);
	Processor_SetAccumulator(processTable[PID].copyOfAccumulatorRegister);
	
	// Same thing for the MMU registers
	MMU_SetBase(processTable[PID].initialPhysicalAddress);
	MMU_SetLimit(processTable[PID].processSize);
}


// Function invoked when the executing process leaves the CPU 
void OperatingSystem_PreemptRunningProcess() {

	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheREADYState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}


// Save in the process' PCB essential values stored in hardware registers and the system stack
void OperatingSystem_SaveContext(int PID) {
	
	// Load PC saved for interrupt manager
	processTable[PID].copyOfPCRegister=Processor_CopyFromSystemStack(MAINMEMORYSIZE-1);
	
	// Load PSW saved for interrupt manager
	processTable[PID].copyOfPSWRegister=Processor_CopyFromSystemStack(MAINMEMORYSIZE-2);

	// Load Accumulator saved for interrupt manager 
	processTable[PID].copyOfAccumulatorRegister = Processor_CopyFromSystemStack(MAINMEMORYSIZE - 3);
	
}


// Exception management routine
void OperatingSystem_HandleException() {
  
	// Show message "Process [executingProcessID] has generated an exception and is terminating\n"
	OperatingSystem_ShowTime(SYSPROC);
	ComputerSystem_DebugMessage(71,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
	
	OperatingSystem_TerminateProcess();
	OperatingSystem_PrintStatus();
}


// All tasks regarding the removal of the process
void OperatingSystem_TerminateProcess() {
  
	int selectedProcess;
	int previousState;
  	
	previousState = processTable[executingProcessID].state;
	processTable[executingProcessID].state=EXIT;
	OperatingSystem_ShowTime(SYSPROC);
	ComputerSystem_DebugMessage(110, SYSPROC, executingProcessID, programList[processTable[executingProcessID].programListIndex] -> executableName, statesNames[previousState], statesNames[4]);
	
	if (programList[processTable[executingProcessID].programListIndex]->type==USERPROGRAM) 
		// One more user process that has terminated
		numberOfNotTerminatedUserProcesses--;
	
	if (numberOfNotTerminatedUserProcesses==0) {
		if (executingProcessID==sipID) {
			// finishing sipID, change PC to address of OS HALT instruction
			OperatingSystem_TerminatingSIP();
			OperatingSystem_ShowTime(SHUTDOWN);
			ComputerSystem_DebugMessage(99,SHUTDOWN,"The system will shut down now...\n");
			return; // Don't dispatch any process
		}
		// Simulation must finish, telling sipID to finish
		OperatingSystem_ReadyToShutdown();
	}
	// Select the next process to execute (sipID if no more user processes)
	selectedProcess=OperatingSystem_ShortTermScheduler();

	// Assign the processor to that process
	OperatingSystem_Dispatch(selectedProcess);
}

// System call management routine
void OperatingSystem_HandleSystemCall() {
  
	int systemCallID;
	int oldPID;
	int PID;
	int queueId;

	// Register A contains the identifier of the issued system call
	systemCallID=Processor_GetRegisterA();
	
	switch (systemCallID) {
		case SYSCALL_PRINTEXECPID:
			OperatingSystem_ShowTime(SYSPROC);
			// Show message: "Process [executingProcessID] has the processor assigned\n"
			ComputerSystem_DebugMessage(72,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			break;

		case SYSCALL_END:
			OperatingSystem_ShowTime(SYSPROC);
			// Show message: "Process [executingProcessID] has requested to terminate\n"
			ComputerSystem_DebugMessage(73,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			OperatingSystem_TerminateProcess();
			OperatingSystem_PrintStatus();
			break;

		case SYSCALL_YIELD:
			queueId= processTable[executingProcessID].queueID;
			if(numberOfReadyToRunProcesses[queueId] > 0) {
				oldPID = executingProcessID;
				// Obtain ready to run process with greater priority
				PID = readyToRunQueue[queueId][0].info;
				// Check new process has the same priority
				if (processTable[oldPID].priority == processTable[PID].priority) {
					//Show message Process [oldPid] will transfer the control of the processor to process [PID]
					OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
					ComputerSystem_DebugMessage(115, SHORTTERMSCHEDULE, oldPID, programList[processTable[oldPID].programListIndex]->executableName,
						PID, programList[processTable[PID].programListIndex]->executableName);
					// Transfer control
					OperatingSystem_PreemptRunningProcess();
					OperatingSystem_Dispatch(PID);
					OperatingSystem_PrintStatus();
				}
			}
			break;

		case SYSCALL_SLEEP:
			processTable[executingProcessID].whenToWakeUp = abs(Processor_GetAccumulator()) + numberOfClockInterrupts + 1;
			OperatingSystem_SaveContext(executingProcessID);
			OperatingSystem_MoveToTheBlockedState(executingProcessID);
			OperatingSystem_PrintStatus();
			break;
	}
}
	
//	Implement interrupt logic calling appropriate interrupt handle
void OperatingSystem_InterruptLogic(int entryPoint){
	switch (entryPoint){
		case SYSCALL_BIT: // SYSCALL_BIT=2
			OperatingSystem_HandleSystemCall();
			break;
		case EXCEPTION_BIT: // EXCEPTION_BIT=6
			OperatingSystem_HandleException();
			break;
		case CLOCKINT_BIT: //CLOCKINT_BIT=9
			OperatingSystem_HandleClockInterrupt();
			break;
	}

}

void OperatingSystem_PrintReadyToRunQueue() {
	int i;

	OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
	ComputerSystem_DebugMessage(106, SHORTTERMSCHEDULE);

	ComputerSystem_DebugMessage(112, SHORTTERMSCHEDULE);
	if (numberOfReadyToRunProcesses[USERPROCESSQUEUE] == 0)
		ComputerSystem_DebugMessage(114, SHORTTERMSCHEDULE);
	else
	{
		for (i = 0; i < numberOfReadyToRunProcesses[USERPROCESSQUEUE]; i++) {
			if (i == numberOfReadyToRunProcesses[USERPROCESSQUEUE] - 1)
				ComputerSystem_DebugMessage(107, SHORTTERMSCHEDULE, readyToRunQueue[USERPROCESSQUEUE][i],
					processTable[readyToRunQueue[USERPROCESSQUEUE][i].info].priority, "\n");
			else
				ComputerSystem_DebugMessage(107, SHORTTERMSCHEDULE, readyToRunQueue[USERPROCESSQUEUE][i],
					processTable[readyToRunQueue[USERPROCESSQUEUE][i].info].priority, ",");
		}
	}

	ComputerSystem_DebugMessage(113, SHORTTERMSCHEDULE);
	if (numberOfReadyToRunProcesses[DAEMONSQUEUE] == 0)
		ComputerSystem_DebugMessage(114, SHORTTERMSCHEDULE);
	else
	{
		for (i = 0; i < numberOfReadyToRunProcesses[DAEMONSQUEUE]; i++) {
			if (i == numberOfReadyToRunProcesses[DAEMONSQUEUE] - 1)
				ComputerSystem_DebugMessage(107, SHORTTERMSCHEDULE, readyToRunQueue[DAEMONSQUEUE][i],
					processTable[readyToRunQueue[DAEMONSQUEUE][i].info].priority, "\n");
			else
				ComputerSystem_DebugMessage(107, SHORTTERMSCHEDULE, readyToRunQueue[DAEMONSQUEUE][i],
					processTable[readyToRunQueue[DAEMONSQUEUE][i].info].priority, ",");
		}
	}
}

void OperatingSystem_HandleClockInterrupt(){ 
	int i, PID, numberOfProcessToWakeUp;

	OperatingSystem_ShowTime(INTERRUPT);
	numberOfClockInterrupts++;
	ComputerSystem_DebugMessage(120,INTERRUPT,numberOfClockInterrupts);

	for (i = 0; i < numberOfSleepingProcesses; i++) {
		if (processTable[sleepingProcessesQueue[i].info].whenToWakeUp == numberOfClockInterrupts) {
			PID = OperatingSystem_ExtractFromBlocked();
			OperatingSystem_MoveToTheREADYState(PID);
			numberOfProcessToWakeUp++;
			i--;
		}
	}

	if (numberOfProcessToWakeUp > 0) {
		OperatingSystem_PrintStatus();
		OperatingSystem_CheckIfIsNecessaryToChangeProcess();
	}
} 

void OperatingSystem_CheckIfIsNecessaryToChangeProcess() {
	int PIDWithMaxPriority;

	PIDWithMaxPriority = Heap_getFirst(readyToRunQueue[USERPROCESSQUEUE], numberOfReadyToRunProcesses[USERPROCESSQUEUE]);
	if (processTable[PIDWithMaxPriority].priority > processTable[executingProcessID].priority) {
		OperatingSystem_ShowTime(SHORTTERMSCHEDULE);
		ComputerSystem_DebugMessage(121,SHORTTERMSCHEDULE, executingProcessID, programList[processTable[executingProcessID].programListIndex] -> executableName, 
			PIDWithMaxPriority, programList[processTable[PIDWithMaxPriority].programListIndex] -> executableName);
		OperatingSystem_PreemptRunningProcess();
		OperatingSystem_Dispatch(PIDWithMaxPriority);
		OperatingSystem_PrintStatus();
	}
}