#include "./ata_dma.h"

#define BOUNDARY 64*1024
#define SECTOR_SIZE 512.0




void init_ATA_DMA(int addr) {

	base_addr = addr;
}


void DMA_Transfer(int sectors, unsigned int lba, void* buffer, drive_t drive, bool is_write){

	base_addr = drive.port_base == 0x1F0 ? base_addr : base_addr + 0x8;

	double size = sectors * SECTOR_SIZE;
	unsigned short port_base = drive.port_base;
	unsigned int offset = 0;
	unsigned int prd_entries_count = ceil(size/BOUNDARY);
	unsigned int prd_table_size = sizeof(prd_entry) * prd_entries_count;

	prd_entry* prd_table = (prd_entry*)allocPages(prd_table_size);

    identityMapPages(prd_table, prd_table_size, 1, 0, 0, 1);

	// Prepare PRD Table

	for(int i = 0; i < prd_entries_count; i++){

		unsigned short rsvd = (i != prd_entries_count - 1) ? 0x0 : 0x8000;

		prd_entry entry = { buffer + offset, size >= BOUNDARY ? 0 : size, rsvd };

		prd_table[i] = entry;

		size -= BOUNDARY;
		offset += BOUNDARY;
	}
	
	outl((unsigned int)prd_table, base_addr + 0x4); // Send the physical PRDT address to the Bus Master PRDT Register.

	outb(is_write ? 0x0 : 0x8, base_addr); //Set the Read bit in the Bus Master Command Register.

	outb(0x0, base_addr + 0x2); // Clear status

	setPortsATA(lba, sectors, drive);

	if(is_write){

		outb(drive.is_28_bit ? 0xCA : 0x35, port_base + 0x7); // send dma transfer command
	}
	else {

		outb(drive.is_28_bit ? 0xC8 : 0x25, port_base + 0x7); // send dma transfer command
	}

	outb(is_write ? 0x5 : 0xD, base_addr); //Set the Start bit on the Bus Master Command Register.



	while (inb(base_addr + 0x2) & 0x1) { }
if (inb(base_addr + 0x2) & 0x2) {  }
if (inb(base_addr + 0x2) & 0x4) {}

}

unsigned short* read_ATA_DMA(int sectors, unsigned int lba, drive_t drive){

	//alocate buffer

	unsigned int buffer_size = sectors * 512;

	unsigned short* buffer = (unsigned short*)allocPages(buffer_size);

	identityMapPages(buffer, buffer_size, 1, 0, 0, 1);
		
	DMA_Transfer(sectors, lba, buffer, drive, false);


	return buffer;
}


void write_ATA_DMA(int sectors, unsigned int lba, void* buffer, drive_t drive){

	DMA_Transfer(sectors, lba, buffer, drive, true);
}