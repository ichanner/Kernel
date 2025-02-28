#include <stdbool.h>
#include "disk.h"
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


void createDisk(unsigned short port_base, unsigned short disk_id, unsigned int sector_count, int index){

	int* sector_bitmap = (int*)alloc(sector_count/32);

	disk_t disk = {port_base, disk_id, sector_count, sector_bitmap};

	disks[index] = disk;
}

int getSector(int sector_index, int disk_index){

	disk_t disk = disks[disk_index];

	int remainder = sector_index % 32;
    int sector = disk.sector_bitmap[sector_index/32] >> remainder;
    
    return sector;
}	

/*
	
	Use best fit algorithm 

*/

int getNextFreeBlock(int size, int disk_index) {

	return 0;
	
}

void setSectors(int sector_start_index, int sector_end_index, int present, int disk_index){

	disk_t disk = disks[disk_index];

	for(int i = sector_start_index; i < sector_end_index; i++){

		int remainder = i % 32;
    	int entry = disk.sector_bitmap[i/32];

	    if (present == 1){
    
	        disk.sector_bitmap[i/32] = entry | (1 << remainder);
	    }
	    else{
	        
	        disk.sector_bitmap[i/32] = entry &  ~(1 << remainder);
	    }
	}
}


