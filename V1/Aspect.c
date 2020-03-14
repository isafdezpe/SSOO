
struct JoinPoint { 
	void** (*fp) (struct JoinPoint *);
	void ** args;
	int argsCount;
	const char ** argsType;
	 void * (*arg)(int, struct JoinPoint *); 
	 const char * (*argType)(int , struct JoinPoint *); 
	void ** retValue;
	const char * retType;
	const char * funcName ;
	const char * targetName ;
	const char * fileName ;
	const char * kind ;
	void * excep_return ;
};

 struct __UTAC__EXCEPTION {
	void * jumpbuf ;
	unsigned long long int prtValue ;
	int pops;
	struct __UTAC__CFLOW_FUNC {
		int (*func)(int,int) ;
		int val ;
		struct __UTAC__CFLOW_FUNC * next; 
	} * cflowfuncs; 
}; 

extern void __utac__exception__cf_handler_set(void * exception, int (*cflow_func)(int, int), int val) ; 
extern void __utac__exception__cf_handler_free(void * exception);
extern void __utac__exception__cf_handler_reset(void * exception) ; 
extern void * __utac__error_stack_mgt(void * env , int mode, int count) ;

# 1 "MyAspect.c" 
# 1 "<built-in>" 
# 1 "<command-line>" 
# 31 "<command-line>" 
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "MyAspect.c" 
# 5 "Clock.h" 1
void Clock_Update(); 
#line 6 "Clock.h"
int Clock_GetTime(); 
# 2 "MyAspect.c" 2
# 39 "Asserts.h" 1
enum assertList {RMEM_OP=0,RMEM_O1=1,RMEM_O2=2,AMEM_OP=3,AMEM_O1=4,AMEM_O2=5,PC=6,ACC=7,IR_OP=8,IR_O1=9,IR_O2=10,PSW=11,MAR=12,MBR_OP=13,MBR_O1=14,MBR_O2=15,MMU_BS=16,MMU_LM=17,MMU_MAR=18,MMEM_MAR=19,MMBR_OP=20,MMBR_O1=21,MMBR_O2=22,XPID=23,RMEM=24,AMEM=25,MBR=26,MMBR=27,PCB_ST=28,PCB_PC=29,PCB_PR=30}; 
#line 46 "Asserts.h"
typedef struct {int time; int value; char element[10]; int address; 
}ASSERT_DATA; 
#line 49 "Asserts.h"
int Asserts_LoadAsserts(); 
#line 50 "Asserts.h"
void Asserts_CheckAsserts(); 
#line 51 "Asserts.h"
void Asserts_TerminateAssertions(); 
#line 53 "Asserts.h"
extern  ASSERT_DATA *asserts; 
#line 55 "Asserts.h"
extern int MAX_ASSERTS; 
#line 57 "Asserts.h"
extern char ASSERTS_FILE[]; 
#line 59 "Asserts.h"
extern int GEN_ASSERTS; 
#line 5 "MyAspect.c"
 inline void __utac_acc__Aspect__1(void) { 



#line 6 "MyAspect.c"
Clock_Update(); }

 
#line 9 "MyAspect.c"
 inline void __utac_acc__Aspect__2(void) { 



#line 10 "MyAspect.c"
Asserts_CheckAsserts(); }

 
#line 13 "MyAspect.c"
 inline void __utac_acc__Aspect__3(void) { 



#line 14 "MyAspect.c"
Clock_Update(); }

 
#line 17 "MyAspect.c"
 inline void __utac_acc__Aspect__4(void) { 



#line 18 "MyAspect.c"
Asserts_TerminateAssertions(); }

 



