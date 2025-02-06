#include <stdbool.h>
#include "./screen.c"
#include "./process.c";
#include "./interrupts/interrupt_handlers.c"

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

	//disable_interrupts();

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



	initScheduler();


//	enable_interrupts();


    while (1) { }
}
 



