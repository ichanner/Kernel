#include <stdbool.h>
#include "./screen.c"
#include "./process.c";
#include "./interrupts/interrupt_handlers.c"
#include "./paging.c"
#include "./ata.c"

void test(){

	print("test");
}

void processA(){


	print("PROCESS A EXECUTED");

	while(1) {

		//print("a");
		asm ( "mov $1, %edi" );

	}
}


void processB(){


	print("Process B");

	while(1) {


		//print("b");
		asm ( "mov $2, %edi" );
	}
}


void processC(){

	print("PROCESS C EXECUTED");

	while(1) {

		//print("c");
		
		asm ( "mov $3, %edi" );
	}
}


void processD(){

	print("PROCESS D EXECUTED");

	while(1) {

		//print("d");

		asm ( "mov $4, %edi" );
	}
}

void processE(){

	print("PROCESS E EXECUTED");

	while(1) {

		//print("e");

		asm ( "mov $5, %edi" );
	}
}


void main() {


	char* msg = "Welcome to Channer OS\0";

	print(msg);	
	println();

	ProcessControlBlock pcbA;
	ProcessControlBlock pcbB;
	ProcessControlBlock pcbC;
	ProcessControlBlock pcbD;
	ProcessControlBlock pcbE;

	createProcess(&processA, &pcbA);
	createProcess(&processB, &pcbB);
	createProcess(&processC, &pcbC);
	createProcess(&processD, &pcbD);
	createProcess(&processE, &pcbE);
	
	//initScheduler();
	
	initPaging();


/*
	for(int i = 0; i < 10; i++){
	
		int* memory = (int*)alloc(4096);

		memory[4095] = 20;

		//memory[4095] = 20;

		println();
		printi(memory[4095]);  // Access the first entry of the second page table
	}
	*/

	//disable_interrupts();

	initATA();

	unsigned short* write_buffer = (unsigned short*)alloc(512);


	//writeATA(1, 50, write_buffer, 0);

	unsigned short* result = readATA(1, 50, 0);

	//	initDisk();


//		clearScreen();

	//2074
	//7573
	//286D
	//7D0A



	for(int i = 0; i < 256; i++){

		printi(result[i]);

		println();
	}
	
	//enable_interrupts();



//	int* memory = (int*)alloc(800);

//	printi(memory[0]);

	//unsigned short* buffer = read(100, 50);




    while (1) { }
}
 



