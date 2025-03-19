
/*
    
    DMA setup



*/


#ifdef thing 

#define SECTOR_SIZE 512.0
#define PAGE_SIZE 4096

typedef struct {

	unsigned int phys_memory;
	unsigned short bytes;
	unsigned short rsvd;

} prd_entry;

int bus_master_register;

/*
void sendReadCmd(unsigned int lba, disk_t disk){

	unsigned short port_base = disk.port_base;

	if(disk.is_28_bit){

		//write number of sectors
		outb(PAGE_SIZE/SECTOR_SIZE, port_base + 2);

		// set the low address of lba bits 0-7
		outb(lba, port_base + 3); 

		// set the mid address of lba bits 8-15
		outb(lba >> 8, port_base + 4);  

		// set the high address of bits 16-23
		outb(lba >> 16, port_base + 5); 

		// Read dma command
		outb(0xC8, port_base + 7);


	}
}*/

void sendReadCmd(unsigned int lba, disk_t disk) {
    
    unsigned short port_base = disk.port_base;


    if (disk.is_28_bit) {
        print("Sending ATA Read Command...");
        println();

        // Ensure drive is ready before issuing command
        unsigned char status = inb(port_base + 7);
        if (status & 0x80) { // Check if BSY (bit 7) is set
            print("ERROR: Drive is busy! Status: ");
            printi(status);
            println();
            return;
        }

        // Write number of sectors
        outb(1, port_base + 2);
        print("Sectors to read: ");
        printi(PAGE_SIZE / SECTOR_SIZE);
        println();

        // Set LBA Low (bits 0-7)
        outb(lba, port_base + 3);
        print("LBA Low: ");
        printi(lba & 0xFF);
        println();

        // Set LBA Mid (bits 8-15)
        outb(lba >> 8, port_base + 4);
        print("LBA Mid: ");
        printi((lba >> 8) & 0xFF);
        println();

        // Set LBA High (bits 16-23)
        outb(lba >> 16, port_base + 5);
        print("LBA High: ");
        printi((lba >> 16) & 0xFF);
        println();

        // Send Read DMA command
        outb(0xC8, port_base + 7);
        print("Read DMA command (0xC8) sent!");
        println();

        // Verify drive accepted the command
        status = inb(port_base + 7);
        if (status & 0x01) { // Check ERR (bit 0)
            print("ERROR: Drive reported an error! Status: ");
            printi(status);
            println();
        }
    }

}

void selectDrive(unsigned int lba, disk_t disk){

	if(disk.is_28_bit){

		// bits 24-27 of lba
		unsigned int msb_lba = ((lba & 0xF0FFFFFF) ^ lba) >> 24;

		/* 
			1110 0000 = 0xE0  master
			1111 0000 = 0xF0  slave

			lower 4 bits for msb_lba
		*/
		outb( msb_lba | (disk.disk_id == 0xA0 ? 0xE0 : 0xF0), disk.port_base + 6);
	}
}







void read(int sectors, unsigned int lba, char* buffer, disk_t disk) {



	bus_master_register = disk.port_base == 0x1F0 ? 0xc040 : 0xc040 + 0x8;


    outb(0x03, disk.port_base + 1); // Feature: Set transfer mode
    outb(0x20, disk.port_base + 2); // Multi-word DMA mode 0 (example)
    outb(0xEF, disk.port_base + 7); // SET FEATURES command
  

    while (inb(disk.port_base + 7) & 0x80) {} // Wait for BSY clear

       
	// total size needed in bytes
	double size = sectors * SECTOR_SIZE;

	// num of prd entries
	int prd_entries_count = ceil(size/PAGE_SIZE);

	// allocate prd table
	
    //prd_entry* prd_table = (prd_entry*)alloc(sizeof(prd_entry) * prd_entries_count);

    prd_entry* prd_table = (prd_entry*)allocContigousFrames(1);

	//Send the physical PRDT address to the Bus Master PRDT Register.
	//outl(prd_table_addr, bus_master_register + 0x4);
	// bit 0: set to 1 for dma mode, bit 3: set to 1 for read direction
	//outb(0x9, bus_master_register);
	//selectDrive(lba, disk);

    /*
	for(int i = 0; i < prd_entries_count; i++){

		// get physical address of current buffer location
		unsigned int phys_addr = virtAddressToPhysAddress(buffer + (i * PAGE_SIZE));

		// if not the last entry set it 0 else msb is 1
		unsigned short rsvd = (i != prd_entries_count - 1) ? 0x0 : 0x8000;

		// create prd entry
		prd_entry entry = { phys_addr, 512, rsvd }; 
		
		prd_table[i] = entry;
		
		sendReadCmd(lba, disk);		

		lba += PAGE_SIZE/SECTOR_SIZE;
	}
    */

    //Prepare a PRDT in system memory.

    prd_entry entry = { buffer, 64*1024, 0x0000 };
    prd_entry entry2 = { buffer + 64*1024, 512, 0x8000 }; 

	prd_table[0] = entry;
    prd_table[1] = entry2;

    //Send the physical PRDT address to the Bus Master PRDT Register.
    outl((unsigned int)prd_table, bus_master_register + 0x4);

    //Set the direction of the data transfer by setting the Read/Write bit in the Bus Master Command Register.
    outb(0x8, bus_master_register);

    //Clear the Error and Interrupt bit in the Bus Master Status Register.
    outb(0x6, bus_master_register + 0x2);

    //Select the drive.
    unsigned int MSB_LBA = ((lba & 0xF0FFFFFF) ^ lba) >> 24;
    outb(MSB_LBA | (disk.disk_id == 0xA0 ? 0xE0 : 0xF0), disk.port_base + 6); 
   
    //Send the LBA and sector count to their respective ports.
    outb(sectors, disk.port_base + 2);
    outb(((lba & 0xFFFFFF00) ^ lba), disk.port_base + 3); // set the low address of lba bits 0-7
    outb(((lba & 0xFFFF00FF) ^ lba) >> 8, disk.port_base + 4); // set the mid address of lba bits 8-15 
    outb(((lba & 0xFF00FFFF) ^ lba) >> 16, disk.port_base + 5); // set the high address of bits 16-23
    
    //Send the DMA transfer command to the ATA controller.
    outb(0xC8, disk.port_base + 7);

    //Set the Start/Stop bit on the Bus Master Command Register.
    outb(0x9, bus_master_register);

    pollATA(false, disk.port_base);

    unsigned char status = inb(bus_master_register + 0x2);

    print("status dma : ");
    printi(status);
    

    unsigned int status_ata = inb(disk.port_base + 7);

    println();
    print("status ata : ");
    printi(status_ata);
    println();

}

void completeAtaTransaction(){

	print("recieved ata finish!");
	println();

	// Set the Start/Stop bit on the Bus Master Command Register.

	outb(0x0, bus_master_register);
}

#endif