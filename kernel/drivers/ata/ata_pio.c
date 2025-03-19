
unsigned short* read_ATA_PIO(int sectors, unsigned int lba, drive_t drive){

	unsigned short port_base = drive.port_base;

	setPortsATA(lba, sectors, drive);

	outb(0x20, port_base + 7); // send read command 0x20

	pollATA(false, port_base);

	unsigned short* buffer = (unsigned short*)alloc(sectors * 512);
 	
 	// read 16 bits per time from 0x1F0

	for(int j = 0; j < (sectors * 512) / 2 ; j++) { 

		buffer[j] = inw(port_base);
	}

	return buffer;
}


void write_ATA_PIO(int sectors, unsigned int lba, void* buffer, drive_t drive){

	unsigned short port_base = drive.port_base;

	setPortsATA(lba, sectors, drive);

	outb(0x30, port_base + 7); // send write command 0x30

	pollATA(false, port_base);

	unsigned short* buffer_wrd = (unsigned short*)buffer;

	for(int j = 0; j < (sectors*512)/2; j++){

		unsigned short word = buffer_wrd[j];

		outw(word, port_base);
	}

	outb(0xE7, port_base + 7); 	// flush the hardware write cache

	pollATA(true, port_base);
}
