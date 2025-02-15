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

/*

Full plan:

enter prtected mode
extern some bootstraped page directory that points to a page tables that points to kernel code oh i need to just map up until the processes are created
load that into cr3 
enable paging by setting 31st bit in cr0 to 1
create page directory entry struct in proccess.c (4 bytes)
create page table etnry struct in proccess.c (4 bytes)
statically allocate an array of size 1024 of page directory entries
statically allocate an array of size 1024 of page table entries
in paging.c create a mapping function that maps page directory entries to page table memory addresses and also map page table entries to dynamically allocated memories (frams)
on irq_32 flush the tlb and load/save the context registers more importantly load cr3 procceses new page direcotey base address
*/

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

		open_processes[process_count] = pcb;

		process_count++;

	}
	else{

		//some error
	}

}





