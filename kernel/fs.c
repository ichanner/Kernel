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



int getSector(int sector_index, drive_t drive){

	int remainder = sector_index % 32;
	int* bmap = drive.sector_bitmap;

    int sector = (bmap[sector_index/32] >> remainder) & 0x1;
    
    return sector;
}	

int getFreeRegionExtent(int curr_sector, drive_t drive){

	int free_sectors = 1;
	int free_sector_index = curr_sector + 1;

	while(getSector(free_sector_index, drive) != 1 && free_sector_index < drive.sector_count) {

		free_sectors++;
		free_sector_index++;
	}

	return free_sectors;
}

int findFirstFreeSector(drive_t drive){

	int curr_sector = 50; // temporary 

	while(true){

		if(getSector(curr_sector, drive) == 0){

			return curr_sector;
		}

		curr_sector += 1;
	}

	
}

int findContinousRegion(int sectors_needed, drive_t drive) {

	int curr_ptr = drive.data_region; // sector index tracker
	int best_fit_ptr = -1;
	int best_fit_free_region = -1;

	while(1) {

		// if the curr sector index is beyond physical capacity stop looking

		if(curr_ptr >= drive.sector_count) {

			break;
		}
		else if(getSector(curr_ptr, drive) == 0){

			// when a free sector is found, try to see how far this free region extend
			
			int free_region = getFreeRegionExtent(curr_ptr, drive);

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

void addEntries(inode_t* inode, int free_sectors, int starting_ptr, drive_t drive){

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

					write_ATA_PIO(1, entry_table_sector, entries, drive);
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
			int new_entry_table_sector = findFirstFreeSector(drive);


			println();
			print("Entry table allocated at sector ");
			printi(new_entry_table_sector);
			println();

			// write entry table to the sector
			write_ATA_PIO(1, new_entry_table_sector, entry_table, drive);

			//set as used
			setSectors(new_entry_table_sector, new_entry_table_sector + 1, 1, drive);

			// assign last entry to new entryly table
			entries[num_entries - 1].blocks = 1;
			entries[num_entries -1].start = new_entry_table_sector;

			//update table in disk if its an extent table to point at tail
			if(entry_table_sector != -1){

				write_ATA_PIO(1, entry_table_sector, entries, drive);
			}

			return;
		}
		else { 

		    entry_table_sector = entries[num_entries - 1].start;

			entries = (entry_t*)read_ATA_PIO(1, entry_table_sector, drive);
		}
		
		// update number of entries we have to look over 


		num_entries = BLOCK_SIZE/sizeof(entry_t);


	}

}


inode_t* allocateBlocks(int size, drive_t drive){

	// 1. Check if there's enough free sectors to store the file


	int avaliable_sectors = 0;
	int sectors_needed = ceil(size/512.0);

	for(int i = drive.data_region; i <  drive.sector_count; i++){

		if(getSector(i, drive) == 0){

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

	int continous_region_ptr = findContinousRegion(sectors_needed, drive);

	if(continous_region_ptr == -1){

		// 3. A continous free region wasn't found. Use first fit to fill in fragments 

		int sectors_left = sectors_needed;
		int curr_ptr = drive.data_region;

		print("Couldn't find continous region");

		while(1){

			if(sectors_left <= 0){

				// 5. break once all sectors are accounted for

				break;
			}
			else if(getSector(curr_ptr, drive) == 0){

				// 4. get the extent of the free region once a free sector is found 

				int free_sectors = getFreeRegionExtent(curr_ptr, drive);

				println();
				print(" Free sectors ");
				printi(free_sectors);

				// 5. Add entries to the inode

				addEntries(inode, free_sectors, curr_ptr, drive);

				// 6. set inode blocks and increment/decrement

				sectors_left -= free_sectors;
				curr_ptr += free_sectors;
			}
			else{

				curr_ptr += 1;
			}
		}
	}
	else {

		print("Found continous region");

		// 3. add entries 

		addEntries(inode, sectors_needed, continous_region_ptr, drive);
	}

	//write inode to disk 

	int sector_for_inode = findFirstFreeSector(drive);

	write_ATA_PIO(1, sector_for_inode, inode, drive);

	println();
	print("Inode allocated at sector ");
	printi(sector_for_inode);
	println();


	setSectors(sector_for_inode, sector_for_inode + 1, 1, drive);


	return inode;

}


/*
void createFile(char* data, int size, char* file_name, drive_t drive){

	// 1. Check if there's enough free sectors to store the file

	int avaliable_sectors = 0;
	int sectors_needed = ceil(size/512.0);

	for(int i = drive.data_region; i <  drive.sector_count; i++){

		if(getSector(i, drive) == 0){

			avaliable_sectors++;
		}
	}

	if(avaliable_sectors < sectors_needed) {

		//ERROR: not enough disk space

		return;
	}
	
	//allocate inode and meta and populate meta fields

	inode_t* inode = (inode_t*)alloc(sizeof(inode_t));
	meta_t* file_meta = (meta_t*)alloc(sizeof(meta_t));

	file_meta->name = file_name;
	file_meta->size = size;
	inode->file_meta = file_meta;
	
	// 2. See if we can find a contious region of free space using best fit

	int continous_region_ptr = findContinousRegion(sectors_needed, drive);

	if(continous_region_ptr == -1){

		// 3. A continous free region wasn't found. Use first fit to fill in fragments 

		int sectors_left = sectors_needed;
		int curr_ptr = drive.data_region;

		print("Couldn't find continous region");

		while(1){

			if(sectors_left <= 0){

				// 5. break once all sectors are accounted for

				break;
			}
			else if(getSector(curr_ptr, drive) == 0){

				// 4. get the extent of the free region once a free sector is found 

				int free_sectors = getFreeRegionExtent(curr_ptr, drive);

				println();
				print(" Free sectors ");
				printi(free_sectors);


				writeATA(curr_ptr,  );

				// 5. Add entries to the inode

				addEntries(inode, free_sectors, curr_ptr, drive);

				// 6. set inode blocks and increment/decrement

				sectors_left -= free_sectors;
				curr_ptr += free_sectors;
			}
			else{

				curr_ptr += 1;
			}
		}
	}
	else {

		print("Found continous region");

		// 3. add entries 

		addEntries(inode, sectors_needed, continous_region_ptr, drive);
	}

	//write inode to disk 

	int sector_for_inode = findFirstFreeSector(drive);

	writeATA(1, sector_for_inode, inode, drive);

	println();
	print("Inode allocated at sector ");
	printi(sector_for_inode);
	println();

	setSectors(sector_for_inode, sector_for_inode + 1, 1, drive);

}
*/



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
	
	
	//allocateBlocks(1024, drives[0]);

//	unsigned short* result = readATA(1, 50, drives[0]);

//	inode_t* test = (inode_t*)result;

//	println();
//	printi(test->entries[0].blocks);
//	println();
//	printi(test->entries[0].start);
//	println();

	setSectors(60, 100, 1, drives[0]);
	

	setSectors(60, 61, 0, drives[0]);
	setSectors(65, 66, 0, drives[0]);

	setSectors(70, 71, 0, drives[0]);
	setSectors(75, 77, 0, drives[0]);

	setSectors(80, 81, 0, drives[0]);
	setSectors(85, 86, 0, drives[0]);

	setSectors(90, 91, 0, drives[0]);

	setSectors(95, 96, 0, drives[0]);


	//leaving us with 7 free sectors 
/*
	print("Meta Bitmap: ");

	for(int i = 50; i < 60; i++){

		printi(getSector(i, drives[0]));
		print(" ");
	}

	print("Data Bitmap: ");

	for(int i = 60; i < 100; i++){

		printi(getSector(i, drives[0]));
		print(" ");
	}
	*/


	// Test 2. 1 entry tables

	allocateBlocks(512*8, drives[0]);


	print("Meta Bitmap: ");

	for(int i = 50; i < 60; i++){

		printi(getSector(i, drives[0]));
		print(" ");
	}

	print("Data Bitmap: ");

	for(int i = 60; i < 100; i++){

		printi(getSector(i, drives[0]));
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



void setSectors(int sector_start_index, int sector_end_index, int present, drive_t drive){


	for(int i = sector_start_index; i < sector_end_index; i++){

		int remainder = i % 32;
    	int entry = drive.sector_bitmap[i/32];

	    if (present == 1){
    
	        drive.sector_bitmap[i/32] = entry | (1 << remainder);
	    }
	    else{
	        
	        drive.sector_bitmap[i/32] = entry &  ~(1 << remainder);
	    }
	}
}

