/*
void createProcess(void* base){

	processContext* context;

	context->eax = 0;
	


	int t = (int)base;

	asm volatile("jmp %0" : : "r"(t));

	printi(t);
	println();
}
*/
#define MAX_PROCESSES 15

//process state

typedef enum { 

	TERMINATED,
	READY, 
	WAITING, 
	RUNNING 

} ProcessState;

//process_context

typedef struct {

	int eax;
	int ebx;
	int ecx;
	int edx;
	int eip;
	int esp;
	int ebp;
	int edi;
	int esi;

} ProcessContext;

//pcb structure 

typedef struct {

	int pci;
	ProcessContext context;
	ProcessState state;

} ProcessControlBlock;


//number of processes
int process_count = 0;

//list of processes initalized to 0 
ProcessControlBlock* open_processes[MAX_PROCESSES];


void createProcess(void* process_base_address, ProcessControlBlock* pcb){

	if(MAX_PROCESSES > process_count){



		pcb->context.eax = 0;
		pcb->context.ebx = 0;
		pcb->context.ecx = 0;
		pcb->context.edx = 0;
		pcb->context.eip = (int)process_base_address;
		pcb->context.esp = 0;
		pcb->context.ebp = 0;
		pcb->context.edi = 0;	
		pcb->pci = process_count;
		pcb->state = READY;

	//	printi((int)process_base_address);

	//	println();

		open_processes[process_count] = pcb;

	//	print("New process created: ");
	//	printi(process_count);
//		println();

		process_count++;

	}
	else{

		//some error
	}

}





