#include <stdbool.h>
#include "fs.h"
#include "math.c"
/*
	TODO:
	- filesystem
		- create file (in progress)
			- sector allocation
			- write file meta info to inode
			- write file contents
		- delete file
		- list files 
		- edit files (difficult)
	- heap free functionality 
	- per process paging
		- cr3 context switch
		- demand paging (difficult)
	- finish LRU swapping
	- cpu threads per process
	- heap manangement per procss
	- stack management per process
	- mouse interrupt and tracking
	- gui terminal 
	- maybe go back in and complete error handling?

	- PHASE 1: Complete , phase 2 will be the network stack

	- Subsequent phases:
		- Full GUI support w/ multiple applications
		- Code compiler 
		- Distrubuted Computing support
		- Eventually make 64 bit supported
		- Look into a RISC fork 
*/



int getSector(int sector_index, disk_t disk){

	int remainder = sector_index % 32;
	int* bmap = disk.sector_bitmap;

    int sector = (bmap[sector_index/32] >> remainder) & 0x1;
    
    return sector;
}	

int getFreeRegionExtent(int curr_sector, disk_t disk){

	int free_sectors = 1;
	int free_sector_index = curr_sector + 1;

	while(getSector(free_sector_index, disk) != 1 && free_sector_index < disk.sector_count) {

		free_sectors++;
		free_sector_index++;
	}

	return free_sectors;
}

int findFirstFreeSector(disk_t disk){

	int curr_sector = 50; // temporary 

	while(true){

		if(getSector(curr_sector, disk) == 0){

			return curr_sector;
		}

		curr_sector += 1;
	}

	
}

int findContinousRegion(int sectors_needed, disk_t disk) {

	int curr_ptr = disk.data_region; // sector index tracker
	int best_fit_ptr = -1;
	int best_fit_free_region = -1;

	while(1) {

		// if the curr sector index is beyond physical capacity stop looking

		if(curr_ptr >= disk.sector_count) {

			break;
		}
		else if(getSector(curr_ptr, disk) == 0){

			// when a free sector is found, try to see how far this free region extend
			
			int free_region = getFreeRegionExtent(curr_ptr, disk);

			// if this free extent can fit what we want and it's smaller than best_fit_free_region then update
			if(free_region >= sectors_needed && (best_fit_free_region < 0 || best_fit_free_region > free_region)){

				best_fit_free_region = free_region;
				best_fit_ptr = curr_ptr;
			}
		
			curr_ptr += free_region;
		}
		else{

			curr_ptr += 1; 
		}
	}

	return best_fit_ptr;
	
}

void addEntries(inode_t* inode, int free_sectors, int starting_ptr, disk_t disk){

	//for initial inode
	int num_entries = (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t); 
	int entry_table_sector = -1; // inode is already written to disk, no need to write it again

	entry_t* entries = inode->entries;

	while(true) { 

		print(" Num entries: ");
		printi(num_entries);
		print(" ");

		for(int i = 0; i < num_entries - 1; i++) {

			if(entries[i].blocks == 0){

				 //avaliable entry found

				print(" ");

				print("Extent entry found at ");

				printi(i);

				print(" ");

				entries[i].blocks = free_sectors;
				entries[i].start = starting_ptr;

				// update this entry table in disk
				if(entry_table_sector != -1) {

					writeATA(1, entry_table_sector, entries, disk);
				}

				return;
			}
		}

		println();
		print("Going to next table");

		// if next entry table doesn't exist in last entry
		if(entries[num_entries - 1].blocks == 0) {

			// allocate new entry table
			entry_table_t* entry_table = (entry_table_t*)alloc(sizeof(entry_table_t));

			// asign first entry in table
			entry_table->entries[0].blocks = free_sectors;
			entry_table->entries[0].start = starting_ptr;

			// find a free sector for it
			int new_entry_table_sector = findFirstFreeSector(disk);


			println();
			print("Entry table allocated at sector ");
			printi(new_entry_table_sector);
			println();

			// write entry table to the sector
			writeATA(1, new_entry_table_sector, entry_table, disk);

			//set as used
			setSectors(new_entry_table_sector, new_entry_table_sector + 1, 1, disk);

			// assign last entry to new entryly table
			entries[num_entries - 1].blocks = 1;
			entries[num_entries -1].start = new_entry_table_sector;

			//update table in disk if its an extent table to point at tail
			if(entry_table_sector != -1){

				writeATA(1, entry_table_sector, entries, disk);
			}

			return;
		}
		else { 

		    entry_table_sector = entries[num_entries - 1].start;

			entries = (entry_t*)readATA(1, entry_table_sector, disk);
		}
		
		// update number of entries we have to look over 


		num_entries = BLOCK_SIZE/sizeof(entry_t);


	}

}


inode_t* allocateBlocks(int size, disk_t disk){

	// 1. Check if there's enough free sectors to store the file


	int avaliable_sectors = 0;
	int sectors_needed = ceil(size/512.0);

	for(int i = disk.data_region; i <  disk.sector_count; i++){

		if(getSector(i, disk) == 0){

			avaliable_sectors++;
		}
	}

	if(avaliable_sectors < sectors_needed) {

		//ERROR: not enough disk space

		return;
	}
	

	inode_t* inode = (inode_t*)alloc(sizeof(inode_t));

	//inode->a = "Hello";


	
	// 2. See if we can find a contious region of free space using best fit

	int continous_region_ptr = findContinousRegion(sectors_needed, disk);

	if(continous_region_ptr == -1){

		// 3. A continous free region wasn't found. Use first fit to fill in fragments 

		int sectors_left = sectors_needed;
		int curr_ptr = disk.data_region;

		print("Couldn't find continous region");

		while(1){

			if(sectors_left <= 0){

				// 5. break once all sectors are accounted for

				break;
			}
			else if(getSector(curr_ptr, disk) == 0){

				// 4. get the extent of the free region once a free sector is found 

				int free_sectors = getFreeRegionExtent(curr_ptr, disk);

				println();
				print(" Free sectors ");
				printi(free_sectors);

				// 5. Add entries to the inode

				addEntries(inode, free_sectors, curr_ptr, disk);

				// 6. set inode blocks and increment/decrement

				sectors_left -= free_sectors;
				curr_ptr += free_sectors;
			}
			else{

				curr_ptr += 1;
			}
		}
	}
	else{

		print("Found continous region");




		// 3. add entries 

		addEntries(inode, sectors_needed, continous_region_ptr, disk);
	}

	//write inode to disk 

	int sector_for_inode = findFirstFreeSector(disk);

	writeATA(1, sector_for_inode, inode, disk);

	println();
	print("Inode allocated at sector ");
	printi(sector_for_inode);
	println();


	setSectors(sector_for_inode, sector_for_inode + 1, 1, disk);


	return inode;

}



void test_fs(){

	clearScreen();

	/*
		
		Test continous File
		Test File with 2 entries 
		Test a file that extends into a entry table 
		Test a file that extends into two entry tables 
		Test a too big file

	*/


	/*
		
		50 - 60: metadata

		60 - 100: data 

	*/

	//Test 1. 1kB file
	
	
	//allocateBlocks(1024, disks[0]);

//	unsigned short* result = readATA(1, 50, disks[0]);

//	inode_t* test = (inode_t*)result;

//	println();
//	printi(test->entries[0].blocks);
//	println();
//	printi(test->entries[0].start);
//	println();

	setSectors(60, 100, 1, disks[0]);
	

	setSectors(60, 61, 0, disks[0]);
	setSectors(65, 66, 0, disks[0]);

	setSectors(70, 71, 0, disks[0]);
	setSectors(75, 77, 0, disks[0]);

	setSectors(80, 81, 0, disks[0]);
	setSectors(85, 86, 0, disks[0]);

	setSectors(90, 91, 0, disks[0]);

	setSectors(95, 96, 0, disks[0]);


	//leaving us with 7 free sectors 
/*
	print("Meta Bitmap: ");

	for(int i = 50; i < 60; i++){

		printi(getSector(i, disks[0]));
		print(" ");
	}

	print("Data Bitmap: ");

	for(int i = 60; i < 100; i++){

		printi(getSector(i, disks[0]));
		print(" ");
	}
	*/


	// Test 2. 1 entry tables

	allocateBlocks(512*8, disks[0]);


	print("Meta Bitmap: ");

	for(int i = 50; i < 60; i++){

		printi(getSector(i, disks[0]));
		print(" ");
	}

	print("Data Bitmap: ");

	for(int i = 60; i < 100; i++){

		printi(getSector(i, disks[0]));
		print(" ");
	}



	// Test 3. 2 entry tables 


	// Test 4. File too big

	/*
	for(int i = 0; i < 256; i++){

		printi(result[i]);

		print(" ");
	}
	*/


}



void setSectors(int sector_start_index, int sector_end_index, int present, disk_t disk){


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


