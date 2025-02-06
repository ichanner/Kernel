

	int running_process_pic;
	int scheduled_process_pic;

	void initScheduler(){

		if(process_count == 0){

			return;
		}

		int eip = open_processes[0]->context.eip;

		running_process_pic = 0;
		scheduled_process_pic = 0;

		asm volatile("jmp %0" : : "r"(eip));
	}


	//precondtion: more than 1 process is avaliable 
	//TODO: address issue when no processes are READY 
	int getNextProcess(){

		//round robin algorithm

		int i = running_process_pic;

		while(true) {

			if(i >= MAX_PROCESSES){

				i = 0; //wrap around when end of structure reached
			}
			else{

				i = i + 1; // otherwise, look at next entry in list
			}
		
			if(open_processes[i]->state != READY){

				continue;
			}
			else{

				return i; //else reutrn it as new pid
			}
		}
	}


	void contextSwitch(){

		if(process_count == 0){

			return;
		}

		int eip = open_processes[scheduled_process_pic]->context.eip;

		asm volatile(

			"sti \n"
			"jmp %0" : : "r"(eip)

		);
	}


	void scheduleProcess(int eax, int ebx, int ecx, int edx, int edi, int eip, int esi){

		//make sure there is a process to switch to 


		if(process_count <= 1){

			return;
		}


		//make a copy of current registers before switching

		open_processes[running_process_pic]->context.eax = eax;
		open_processes[running_process_pic]->context.ebx = ebx;
		open_processes[running_process_pic]->context.ecx = ecx;
		open_processes[running_process_pic]->context.edx = edx;
		open_processes[running_process_pic]->context.edi = edi;
		open_processes[running_process_pic]->context.esi = esi;
		open_processes[running_process_pic]->context.eip = eip;

		//find next pic 

		int next_pic = getNextProcess();


		//change state of current process 

		open_processes[running_process_pic]->state = READY;


		// update registers with next_pic context

		eax = open_processes[next_pic]->context.eax;
		ebx = open_processes[next_pic]->context.ebx;
		ecx = open_processes[next_pic]->context.ecx;
		edx = open_processes[next_pic]->context.edx;
		esi = open_processes[next_pic]->context.esi;
		edi = open_processes[next_pic]->context.edi;

		asm volatile("mov %0, %%eax" : : "r"(eax));
		asm volatile("mov %0, %%ebx" : : "r"(ebx));
		asm volatile("mov %0, %%ecx" : : "r"(ecx));
		asm volatile("mov %0, %%edx" : : "r"(edx));
		asm volatile("mov %0, %%esi" : : "r"(esi));
		asm volatile("mov %0, %%edi" : : "r"(edi));

		// change state of next_pic 

		open_processes[next_pic]->state = RUNNING;

		// schedule it 
		
		running_process_pic = next_pic;
		scheduled_process_pic = next_pic;

	}
