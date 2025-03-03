#include "disk.h"

void createDisk(unsigned short port_base, unsigned short disk_id, unsigned int sector_count, int index){

	int* sector_bitmap = (int*)alloc(sector_count*32);

	memset(sector_bitmap, sector_count*32, 0);

	//60 is temp
	disk_t disk = {port_base, disk_id, sector_count, 60, sector_bitmap};
	disks[index] = disk;
}