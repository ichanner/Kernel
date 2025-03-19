//Dev debug function
#include "../../pci.c"
#include "./ata_dma.c"

#define DATA_REGION_PORTION 0.6

/*
	
	TODO:

		Error handling 
*/

void driveDebug(unsigned short drive_id, bool is_secondary, char* msg){

	println();

	if(drive_id == 0xA0){

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


void configureDMADevice(){

	unsigned int bus; 
	unsigned int slot;
	unsigned int func;

	enumeratePCI(0x1, 0x1, &bus, &slot, &func);

	//enable bus mastering 
	unsigned int cmd = pciConfigReadDWord(bus, slot, func, 0x4);
	
	cmd |= 0x4;
	
	pciConfigWriteDword(bus, slot, func, 0x4, cmd);

	unsigned int bar4 = pciConfigReadDWord(bus, slot, func, 0x20);

	unsigned interupt = pciConfigReadDWord(bus, slot, func, 0x3C);

	interupt |= 0x100;

	println();

	print(" interupt: ");
	printi(interupt);

	pciConfigWriteDword(bus, slot, func, 0x3c, interupt);


	println();

	if(bar4 & 0x1) {

		// it's port mapped
		init_ATA_DMA(bar4 & 0xFFFFFFFC);

	}
	else{

		// it's memory mapped
		init_ATA_DMA(bar4 & 0xFFFFFFF0);
	}
}

void setDMAMode(unsigned int dma_mode, unsigned int port_base, unsigned char is_udma){

	outb(0x03, port_base + 1); //set transfer mode

	if(is_udma) {

		outb(0x40 + dma_mode, port_base + 2);
	}
	else{

		outb(dma_mode, port_base + 2);
	}

	outb(0xEF, port_base + 7); //set features command 

	while (inb(port_base + 7) & 0x80) {} // wait for BSY to clear
}

void identifyATADrive(bool is_master, bool is_secondary, int id){

	unsigned short port_base = is_secondary ? 0x170 : 0x1F0;

	outb(is_master ? 0xA0 : 0xB0, port_base + 6);

	outb(0, port_base + 2);
	outb(0, port_base + 3);
	outb(0, port_base + 4);
	outb(0, port_base + 5);

	// send identification command to drive
	outb(0xEC, port_base + 7);

	// check status to see if the drive exists 

	unsigned char status = inb(port_base + 7);

	if(status == 0){

		driveDebug(is_master, is_secondary, "Not found!");

		return;
	}

	// poll until BSY clears and check lba_mid and lba_hi

	while(status & 0x80){

		status = inb(port_base + 7);
	}

	unsigned int lba_mid = inb(port_base + 4);
	unsigned int lba_hi = inb(port_base + 5);

	if(lba_mid != 0 || lba_hi != 0){

		driveDebug(is_master, is_secondary, "Not an ATA device!");

		return;
	}

	// poll until DRQ sets or an ERR occurs 

	pollATA(false, port_base);
	
	driveDebug(is_master, is_secondary, "Info ready!");

	short driver_info[256];

	unsigned char is_28_bit = 0;
	unsigned char is_udma = 0;
	unsigned int dma_mode = -1;
	unsigned int sector_count = 0;

	//read driver info one word at a time

	for(int i = 0; i < 256; i += 1){

		driver_info[i] = inw(port_base);
	}

	//find highest udma mode that's supported

	for(int i = 0; i < 8; i++){

		if(driver_info[88] & (1 << i)){

			is_udma = 1;
			dma_mode = i;
		}
	}

	//if no udma, then find highest mwdma mode that's supported

	if(dma_mode == -1) {

		for(int i = 0; i < 3; i++){

			if(driver_info[63] & (1 << i)){

				dma_mode = i;
			}
		}

	}

	if(dma_mode != -1) {

		setDMAMode(dma_mode, port_base, is_udma);
		configureDMADevice();

		driveDebug(is_master, is_secondary, is_udma ? "UDMA mode: " : "MWDMA mode: ");
		printi(dma_mode);

	}
	else {

		driveDebug(is_master, is_secondary, "PIO mode");
	}

	if(driver_info[83] & 0x200) {

		driveDebug(id, is_secondary, "Supports 48 bit addressing");

		unsigned short lower = driver_info[100];
		unsigned int mid = driver_info[102] << 16;

		//TODO: when we do 64-bit system include the 103rd word, cheers
		sector_count = mid | lower;

	}
	else {

		driveDebug(id, is_secondary, "Supports 28 bit addressing");
		
		unsigned short lower = driver_info[60];
		unsigned int upper = driver_info[61] << 16;
			
		is_28_bit = 1;
		sector_count = upper | lower;
	}

	driveDebug(is_master, is_secondary, "Sector Count: ");
	printi(sector_count);

	createATADrive(id, port_base, sector_count, sector_count * DATA_REGION_PORTION, dma_mode, is_master, is_28_bit);
}

void initATA(){

	identifyATADrive(true, false, 0);
	identifyATADrive(false, false, 1);
	identifyATADrive(true, true,  2);
	identifyATADrive(false, true,  3);
}

void setPortsATA(unsigned int lba, unsigned int sector_count, drive_t drive) {

	unsigned int port_base = drive.port_base;

	/*
		1110 0000 = 0xE0  master
		1111 0000 = 0xF0  slave
	*/

	if(drive.is_28_bit){

		unsigned int msb_lba = (lba >> 24) & 0xF; 
		outb(msb_lba | (drive.is_master ? 0xE0 : 0xF0), port_base + 6);
		
		outb(sector_count, port_base + 2); // send sector count
		outb(lba, port_base + 3); //lba mid 7-0
		outb(lba >> 8, port_base + 4); //lba mid 15-8
		outb(lba >> 16, port_base + 5); //lba high 23-16
	}
	else {

		outb((drive.is_master ? 0xE0 : 0xF0), port_base + 6);

		outb(sector_count >> 8 , port_base + 2); // send sector count high byte
		outb(lba >> 24, port_base + 3); //lba low 31-24
		outb(lba >> 32, port_base + 4); //lba mid 39-32
		outb(lba >> 40, port_base + 5); //lba high 47-40

		outb(sector_count, port_base + 2); //send sector count low byte
		outb(lba, port_base + 3); //lba low 7-0
		outb((lba >> 8) & 0xFF, port_base + 4); //lba mid 15-8
		outb((lba >> 16) & 0xFF, port_base + 5); //lba high 23-16
	}

}	


void pollATA(bool no_drq_needed, unsigned short port_base) {

	unsigned int status = inb(port_base + 7);

	while(true) {  

		if(status & 0x01){

			print("DISK ERRR");

			break;
		}
		else if(status & 0x20){

			print("DISK DF");

			break;
		}
		else if(!(status & 0x80) && (no_drq_needed || status & 0x08)){

			break;
		}

		status = inb(port_base + 7);

	}	

	return;

}
