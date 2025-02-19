

unsigned short* read(int sectors, unsigned int LBA){

	//read one sector 0x1F2

	outb(sectors, 0x1F2);

	//lets read sector 50 for fun  LBA 50


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

	//send write command 0x20 to 0x1F7

	outb(0x20, 0x1F7);

	//read status register until 7th bit isn't 1

	unsigned int status = inb(0x1F7);

	while(status == 0x80){

		status = inb(0x1F7);
	}		


	//read 16 bits per time from 0x1F0

	unsigned short* buffer = (unsigned short*)alloc(sectors * 512);

	for(int i = 0; i < sectors; i++){
 
		for(int j = 0; j < 32; j++){ //512/16 = 32

			// read one word from 0x1F0

			print("read");


			buffer[i*j] = inw(0x1F0);

		//	printi(buffer[i*j]);
				
		//	println();
		}
	}	
	

	return buffer;

}


void write(){}