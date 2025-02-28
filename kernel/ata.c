//Dev debug function

#include "./disk.c"

/*
	
	TODO:

		Error handling 
*/

void diskDebug(unsigned short disk_id, bool is_secondary, char* msg){

	println();

	if(disk_id == 0xA0){

		print("Master Drive at ");
	}
	else{

		print("Slave Drive at ");
	}

	if(is_secondary){

		print("Secondary Bus: ");
	}
	else{

		print("Primary Bus:  ");
	}

	print(msg);
}


void identifyATADisk(unsigned short disk_id, bool is_secondary, int index){

	/*
		 identify disk 

		 	0xA0 for master drive
			0xB0 for slave drive
	*/

	outw(disk_id, 0x1F6);

	/*
		Primary or secondary bus?
	*/
	unsigned short port_base = is_secondary ? 0x170 : 0x1F0;

	outb(0, port_base + 2);
	outb(0, port_base + 3);
	outb(0, port_base + 4);
	outb(0, port_base + 5);

	// send identification command to disk
	outw(0xEC, port_base + 7);

	unsigned char status = inb(port_base + 7);

	if(status == 0){

		diskDebug(disk_id, is_secondary, "Not found!");

		return;
	}

	while(status & 0x80){

		status = inb(port_base + 7);
	}

	unsigned int lba_mid = inb(port_base + 4);
	unsigned int lba_hi = inb(port_base + 5);

	if(lba_mid != 0 || lba_hi != 0){

		diskDebug(disk_id, is_secondary, "Not an ATA device!");

		return;
	}

	pollATA(false, port_base);
	
	diskDebug(disk_id, is_secondary, "Info ready!");

	short driver_info[256];

	for(int i = 0; i < 256; i += 1){

		driver_info[i] = inw(port_base);
	}

	// bit 10 of the 83rd word indicates drive supports lba 48 bit addressing

	println();

	unsigned int combined_sector_count;

	if(driver_info[83] & 0x200) {

		diskDebug(disk_id, is_secondary, "Supports 48 bit addressing");

		// get words 100 - 102

		unsigned short lower = driver_info[100];
		unsigned int mid = driver_info[102] << 16;
		
		combined_sector_count = mid | lower;

		diskDebug(disk_id, is_secondary, "Sector Count: ");
		printi(combined_sector_count);
		println();

		//when we do 64 bit system include the 103rd word, cheers
	}
	else {

		diskDebug(disk_id, is_secondary, "Supports 48 bit addressing");
		
		// get word 60 and 61 

		unsigned short lower = driver_info[60];
		unsigned int upper = driver_info[61] << 16;
		
		combined_sector_count = upper | lower;

		diskDebug(disk_id, is_secondary, "Sector Count: ");
		printi(combined_sector_count);
		println();

	}

	createDisk(port_base, disk_id, combined_sector_count, index);
}

void initATA(){

	identifyATADisk(0xA0, false, 0);
	identifyATADisk(0xB0, false, 1);
	identifyATADisk(0xA0, true,  2);
	identifyATADisk(0xB0, true,  3);
}

void pollATA(bool no_drq_needed, unsigned short port_base) {

	//read status register until 7th bit isn't 1 (BSY flag)

	unsigned int status = inb(port_base + 7);

	while(true) {  

/*
		println();
		print("Drive Status");
		printi(status);
		println();
		//println();
*/
		if(status & 0x01){

			//ERR

			print("DISK ERRR");

			break;
		}
		else if(status & 0x20){

			//DF

			print("DISK DF");

			break;
		}
		else if(!(status & 0x80) && (no_drq_needed || status & 0x08)){

		//	print("ready");

			break;
		}

		status = inb(port_base + 7);


	}	

	return;

}

unsigned short* readATA(int sectors, unsigned int LBA, int disk_index){

	disk_t disk = disks[disk_index];

	unsigned short port_base = disk.port_base;
	unsigned short disk_id = disk.disk_id;

	//read number of sectors 

	outb(sectors, port_base + 2);

	outb(((LBA & 0xFFFFFF00) ^ LBA), port_base + 3); // set the low address of lba bits 0-7
	outb(((LBA & 0xFFFF00FF) ^ LBA) >> 8, port_base + 4); // set the mid address of lba bits 8-15 
	outb(((LBA & 0xFF00FFFF) ^ LBA) >> 16, port_base + 5); // set the high address of bits 16-23

	/*
		Bits 0-3 is Bits 24-27 from the Logical Block Address
		Bit 4 determines which drive to use (0=Master, 1=Slave)
		Bit 5 is always 1 
		Bit 6 is set to 1 for LBA mode 0 for CHS mode 
		Bit 7 is always 1 
	*/

	unsigned int MSB_LBA = ((LBA & 0xF0FFFFFF) ^ LBA) >> 24;

	/* 
		1110 0000 = 0xE0  master
		1111 0000 = 0xF0  slave
		
	*/
	outb(MSB_LBA | (disk_id == 0xA0 ? 0xE0 : 0xF0), port_base + 6); 

	//send read command 0x20 to 0x1F7

	outb(0x20, port_base + 7);

	pollATA(false, port_base);

	unsigned char status = inb(port_base + 7); 

	print("ready");

	unsigned short* buffer = (unsigned short*)alloc(sectors * 512);
 	
 	//read 16 bits per time from 0x1F0

	for(int j = 0; j < (sectors * 512) / 2 ; j++) { 

		// read one word from 0x1F0
		buffer[j] = inw(port_base);
	}

	print("finsihed");
	
	return buffer;
	

}

void writeATA(int sectors, unsigned int LBA, unsigned short* buffer, int disk_index){

	disk_t disk = disks[disk_index];

	unsigned short port_base = disk.port_base;
	unsigned short disk_id = disk.disk_id;

	//write number of sectors 
	outb(sectors, port_base + 2);

	outb(((LBA & 0xFFFFFF00) ^ LBA), port_base + 3); // set the low address of lba bits 0-7
	outb(((LBA & 0xFFFF00FF) ^ LBA) >> 8, port_base + 4); // set the mid address of lba bits 8-15 
	outb(((LBA & 0xFF00FFFF) ^ LBA) >> 16, port_base + 5); // set the high address of bits 16-23

	unsigned int MSB_LBA = ((LBA & 0xF0FFFFFF) ^ LBA) >> 24;

	/* 
		1110 0000 = 0xE0  master
		1111 0000 = 0xF0  slave
		
	*/
	outb(MSB_LBA | (disk_id == 0xA0 ? 0xE0 : 0xF0) , port_base + 6); //11100000 = 0xE0 

	//send write command 0x30 to 0x1F7
	outb(0x30, port_base + 7);

	pollATA(false, port_base);

	for(int j = 0; j < sizeof(buffer)/sizeof(buffer[0]); j++){

		unsigned short word = buffer[j];

		outw(word, port_base);
	}

	// flush the hardware write cache

	outb(0xE7, port_base + 7);

	pollATA(true, port_base);


}