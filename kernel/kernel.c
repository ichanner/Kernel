#include <stdbool.h>
#include "./screen.c"
#include "./process.c";
#include "./interrupts/interrupt_handlers.c"

#include "./memory/memory.c"

#include "./string.c"

#include "./drive.c"
#include "./drivers/ata/ata.c"
#include "./fs.c"

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


	char* msg = "Welcome to Channer OS";

	print(msg);	
	println();

/*
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
*/

	initMemory();
	
	initATA();




/*
	print("HELLO");

	println();

   setFrame(10, 1);

   setFrame(45, 1);

   print("he ");
   printi(findFreeFrame());
   println();
   printi(getFrame(259));
*/
   println();

/*
   char* token = "hello,iancc,washh";  

   token = strok("hello,iancc,washh", ','); 
  
   while(token != NULL){

   		print(token);

	   	token = strok(NULL, ',');

	   	print(" ");

   }
   */

   initFs(&drives[0], 1);

   


//	char* write = (char*)kalloc(512);

//	write_ATA_DMA(1, 51, write, drives[0]);




   	//partition_t* table = (partition_t*)readPartitionTable(drives[0]);

   //	printi(table[0].status);

	//test_fs();

 
//	unsigned int r = pciConfigReadDWord(0, 1, 1, 0x4);

//	r |= 0x4;

//	pciConfigWriteDword(0, 1, 1, 0x4, r);

	//test_fs();		

	/*

	char* test = (char*)allocContigousFrames(1);
	
	read(2, 50, test, disks[0]);

	for(int i = 0; i < 512*2; i++){

		printi(test[i]);
	}


	*/
	
/*

	int size = 512*24;
	
	char* buffer = (char*)alloc(size);

	for(int i = 0; i < ceil(size/4096); i++){

		println();

		print("test: ");

		printi(virtAddressToPhysAddress(buffer + (i * 4096)));
	}
*/


/*

	for(int i = 0; i < 1025; i++){
	
		int* memory = (int*)alloc(4096);

		memory[4095] = 2;

	//	print("Phys Address: ");
	//	printi(virtAddressToPhysAddress(memory[4095]));

	//	println();

		print("");
		printi(memory[4095]);  // Access the first entry of the second page table
	}

*/
/*
	unsigned short* write_buffer = (unsigned short*)alloc(512);
	write_buffer[0] = 1;

	writeATA(1, 50, write_buffer, disks[0]);

	unsigned short* result = readATA(1, 50, disks[0]);



	//	initDisk();


//		clearScreen();

	//2074
	//7573
	//286D
	//7D0A


/*
	for(int i = 0; i < 256; i++){

		printi(result[i]);

		println();
	}
	*/
	
	//enable_interrupts();



//	int* memory = (int*)alloc(800);

//	printi(memory[0]);

	//unsigned short* buffer = read(100, 50);


/*


	int* memory = (int*)alloc(4096);

	memory[0] = 5;
	memory[1] = 3;

	printi(virtAddressToPhysAddress(memory)); 

	println();
	printi(memory); 

*/



    while (1) { }
}
 



