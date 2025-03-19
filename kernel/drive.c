#include "drive.h"

drive_t createDrive(unsigned short port_base, unsigned int sector_count, unsigned int data_region){

	int bitmap_size = (sector_count + 31) / 32 * sizeof(int);
	int* sector_bitmap = (int*)alloc(bitmap_size);

	memset(sector_bitmap, bitmap_size, 0);

	drive_t drive = { sector_count, data_region, sector_bitmap };

	return drive;
}

void createATADrive(int id, unsigned short port_base, unsigned int sector_count, unsigned int data_region,
	unsigned int dma_mode, unsigned char is_master, unsigned char is_28_bit){

	drive_t drive = createDrive(port_base, sector_count, data_region);

	drive.port_base = port_base;
	drive.dma_mode = dma_mode;
	drive.is_master = is_master;
	drive.is_28_bit = is_28_bit;
	
	drives[id] = drive;
}