#include "drive.h"


void createATADrive(int id, unsigned short port_base, unsigned int sector_count, unsigned int dma_mode, unsigned char is_master, unsigned char is_28_bit, unsigned short* (*read)(int, unsigned int, struct drive), void (*write)(int, unsigned int, void*, struct drive)){

	drive_t drive = { sector_count, read, write };

	drive.port_base = port_base;
	drive.dma_mode = dma_mode;
	drive.is_master = is_master;
	drive.is_28_bit = is_28_bit;
	
	drives[id] = drive;
}