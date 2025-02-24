#include <stdbool.h>

/*

	ATA write
	Disk swap With Swap Table
	LRU Swap
	Acess Bit Clearing timing routine 
	
	per process paging
		- cr3 context
		-tlb flush 
		-disk lookup table
		-demand paging fault handling
*/



void poll(bool no_drq_needed) {

	//read status register until 7th bit isn't 1 (BSY flag)

	unsigned int status = inb(0x1F7);

	while(true) {  

		status = inb(0x1F7);

		if(status & 0x01 || status & 0x20 ){

			/*
				
				TODO:

					on ERR flag resend command
					on DF flag reset driver

			*/

			print("err");

			return;
		}
		else if(!(status & 0x80) && (no_drq_needed || status & 0x08) ){

			print("ready");

			return;
		}

	}	


}

unsigned short* read(int sectors, unsigned int LBA){

	//read number of sectors 

	outb(sectors, 0x1F2);

	outb(((LBA & 0xFFFFFF00) ^ LBA), 0x1F3); // set the low address of lba bits 0-7
	outb(((LBA & 0xFFFF00FF) ^ LBA) >> 8, 0x1F4); // set the mid address of lba bits 8-15 
	outb(((LBA & 0xFF00FFFF) ^ LBA) >> 16, 0x1F5); // set the high address of bits 16-23

	/*
		Bits 0-3 is Bits 24-27 from the Logical Block Address
		Bit 4 determines which drive to use (0=Master, 1=Slave)
		Bit 5 is always 1 
		Bit 6 is set to 1 for LBA mode 0 for CHS mode 
		Bit 7 is always 1 
	*/

	unsigned int MSB_LBA = ((LBA & 0xF0FFFFFF) ^ LBA) >> 24;

	outb(MSB_LBA | 0xE0 , 0x1F6); // 11100000 = 0xE0 

	//send read command 0x20 to 0x1F7

	outb(0x20, 0x1F7);

	poll(false);

	unsigned short* buffer = (unsigned short*)alloc(sectors * 512);
 	
 	//read 16 bits per time from 0x1F0

	for(int j = 0; j < (sectors * 512) / 2 ; j++) { 

		// read one word from 0x1F0
		buffer[j] = inw(0x1F0);
	}

	print("finsihed");
	
	println();

	return buffer;
	

}


void write(int sectors, unsigned int LBA, unsigned short* buffer){

	//write number of sectors 
	outb(sectors, 0x1F2);

	outb(((LBA & 0xFFFFFF00) ^ LBA), 0x1F3); // set the low address of lba bits 0-7
	outb(((LBA & 0xFFFF00FF) ^ LBA) >> 8, 0x1F4); // set the mid address of lba bits 8-15 
	outb(((LBA & 0xFF00FFFF) ^ LBA) >> 16, 0x1F5); // set the high address of bits 16-23

	unsigned int MSB_LBA = ((LBA & 0xF0FFFFFF) ^ LBA) >> 24;

	outb(MSB_LBA | 0xE0 , 0x1F6); //11100000 = 0xE0 

	//send write command 0x30 to 0x1F7
	outb(0x30, 0x1F7);

	poll(false);

	for(int j = 0; j < sizeof(buffer)/sizeof(buffer[0]); j++){

		unsigned short word = buffer[j];

		outw(word, 0x1F0);
	}

	// flush the hardware write cache

	outb(0xE7, 0x1F7);

	poll(true);

}