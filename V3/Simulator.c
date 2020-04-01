#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Simulator.h"
#include "ComputerSystem.h"
#include "ComputerSystemBase.h"
#include "Asserts.h"

// Functions prototypes
int Simulator_GetOption(char *);


extern int initialPID;
extern int endSimulationTime; // For end simulation forced by time
extern char *debugLevel;
extern int intervalBetweenInterrupts; // For interval between interrupts parameter

char *options[]={
	"--initialPID",
	"--endSimulationTime",
	"--numAsserts",
	"--assertsFile",
	"--debugSections",
	"--intervalBetweenInterrupts",
	"--generateAsserts",
	"--help",
	NULL };

char *optionsDefault[]={
	"0",
	"-1",
	"500",
	"asserts",
	"A",
	"5",
	"No value",
	"No value",
	NULL
};

enum {INITIALPID, ENDSIMULATIONTIME, NUMASSERTS, ASSERTSFILE, DEBUGSECTIONS, INTERVALBETWEENINTERRUPTS, GENERATEASSERTS, HELP};


int main(int argc, char *argv[]) {
  
	// We now have a multiprogrammed computer system
	// No more than PROGRAMSMAXNUMBER in the command line
	int paramIndex=1; // argv index
	int i, rc, numPrograms=0, isOption=1;
	char *option, *optionValue;

	for (i=paramIndex; i < argc && isOption ;) {
		if (argv[i][0]=='-' && argv[i][1]=='-') {
			option=strtok(argv[i],"=");
			optionValue=strtok(NULL," ");
			int optionIndex=Simulator_GetOption(option);
			switch (optionIndex) {
				case INITIALPID:
					if (optionValue==NULL || sscanf(optionValue,"%d",&initialPID)==0) 
							initialPID=PROCESSTABLEMAXSIZE-1;
					break;
				case ENDSIMULATIONTIME:
					if (optionValue==NULL || sscanf(optionValue,"%d",&endSimulationTime)==0)
						endSimulationTime=-1;
					break;
				case NUMASSERTS:
					if (optionValue==NULL) 
						MAX_ASSERTS=500;
					else {
						rc=sscanf(optionValue,"%d",&MAX_ASSERTS);
						if (rc<=0 || MAX_ASSERTS<=0)
							MAX_ASSERTS=500;
					}
					break;
				case ASSERTSFILE:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen("asserts")+1)*sizeof(char));
						strcpy(optionValue,"asserts");
						// LOAD_ASSERTS_CONF=1;
					}
					strcpy(ASSERTS_FILE,optionValue);
					break;
				case DEBUGSECTIONS:
					if (optionValue==NULL){
						optionValue=(char *) malloc((strlen("A")+1)*sizeof(char));
						strcpy(optionValue,"A");
					}
					debugLevel=optionValue;
					break;
				case GENERATEASSERTS:
					GEN_ASSERTS=1; 
					break;
				case INTERVALBETWEENINTERRUPTS:
					if (optionValue==NULL || sscanf(optionValue,"%d",&intervalBetweenInterrupts)==0)
						intervalBetweenInterrupts=DEFAULT_INTERVAL_BETWEEN_INTERRUPTS;
					break;
				case HELP:
					{
						int j;
						printf("Use one or more of these options:\n");
						for (j=0; options[j]!=NULL; j++)
							if (j<GENERATEASSERTS)
								printf("\t%s=ValueOfOption  [%s]\n",options[j], optionsDefault[j]);
							else
								printf("\t%s\n",options[j]);
					}
					break;
				default :
					printf("Invalid option: %s\n", option);
					break;
			}
			i++;
		}
		else isOption=0;
	}
		
	paramIndex=i;
	for (i=paramIndex;i<argc;){
		rc=strncmp(argv[i],"--",2);
		i++;
		if (rc!=0) {
			numPrograms++;
			if ((i < argc)
			 && (sscanf(argv[i], "%d", &rc) == 1))
				// An arrival time has been read. Increment i
				i++;
		}
		else {
			numPrograms=-1; // Options beetwen programs are not allowed
			break;
		}
	}

	if ((numPrograms<=0) || (numPrograms>PROGRAMSMAXNUMBER)) {
		printf("USE: Simulator [--optionX=optionXValue ...] <program1> [arrivalTime] [<program2> [arrivalTime] .... <program%d [arrivalTime]] \n",PROGRAMSMAXNUMBER);
		if (numPrograms<0)
			printf("Options must be before program names !!!\n");
		else 
			printf("Must have beetwen 1 and %d program names !!!\n", PROGRAMSMAXNUMBER);
		exit(-1);
	}



	// The simulation starts
	ComputerSystem_PowerOn(argc, argv, paramIndex);
	// The simulation ends
	ComputerSystem_PowerOff();
	return 0;
}

int Simulator_GetOption(char *option){
	int i;
	for (i=0; options[i]!=NULL ; i++)
		if (strcasecmp(options[i],option)==0)
			return i;
	return -1;
}
