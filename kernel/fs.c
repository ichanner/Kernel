#include <stdbool.h>
#include <stddef.h>
#include "fs.h"
#include "math.c"
#include "./partition.c"

/*
	TODO:
	

	
	- filesystem
		
	- per process paging
		- cr3 context switch
		- demand paging 
	- finish LRU swapping
	
	- cpu threads per process
	- send process signals  
	- heap manangement per procss
	- stack management per process
	


	- mouse interrupt and tracking
	- gui terminal 

	- maybe go back in and complete error handling?

	- PHASE 1: Complete , phase 2 will be the network stack

	- Subsequent phases:
		- More disk/ethernet drivers 
		- Full GUI support w/ multiple applications
		- Code compiler 
		- Eventually make 64 bit supported
*/


void* alloc_buffer(unsigned int size, drive_t drive) {

	if(drive.dma_mode != -1) {

		void* buffer = allocPages(size);

		identityMapPages(buffer, size, 1, 0, 0, 1);

		return buffer;
	}

	return kalloc(size);
}

inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t drive, int partition_index, int sector_for_dir) {

	inode_t* inode = (inode_t*)alloc_buffer(sizeof(inode_t), drive);

	inode->meta.type = DIR;
	inode->meta.name = name;
	inode->meta.size = 0;

	if(sector_for_dir == -1) {

		sector_for_dir = findFirstFreeSector(drive, partition_index);
	}

	drive.write(1, sector_for_dir, inode, drive);

	setSectors(sector_for_dir, sector_for_dir + 1, 1, drive);

	if(parent_directory != NULL) {

		addEntries(NULL, parent_directory, 1, sector_for_dir, drive, partition_index);
	}

	return inode;

}



void initFs(drive_t* drive, int partition_index) {

	partition_entry_t* partition_table = readPartitionTable(*drive);
	superblock_t* superblock = readSuperBlock(partition_table, drive, partition_index);

	if(superblock == NULL){

		if(partition_table == NULL) { // for raw external drives 

			unsigned int data_region = 1 + (drive->sector_count * DATA_META_RATIO);

			updateSuperBlock(1, data_region, drive->sector_count, 0, drive, 0);
			createDirectory(NULL, "root", *drive, 0, drive->partitions[0].root_inode_lba);

			return;
		}

		print("SUPERBLOCK was NOT found");
		println();
	
		int last_end_sector = 0;

		for(int i = 0; i < 4; i++) {

			if(partition_table[i].status != 0x80) {

				partition_table[i].status = 0x80; 
				partition_table[i].start_sector = last_end_sector + 1;
				partition_table[i].size = drive->sector_count - partition_table[i].start_sector;
				partition_table[i].type = 0x83; 

				for(int j = i + 1; j < 4; j++){

					if(partition_table[j].status == 0x80){

						partition_table[i].size -= partition_table[j].size;
					}
				}


				unsigned int meta_region = partition_table[i].start_sector + 1;
				unsigned int data_region = partition_table[i].start_sector + (partition_table[i].size * DATA_META_RATIO);
				unsigned int sector_count = partition_table[i].size;
				unsigned int start_lba = partition_table[i].start_sector;

				updateSuperBlock(meta_region, data_region, sector_count, start_lba, drive, partition_index);
				createDirectory(NULL, "root", *drive, i, drive->partitions[i].root_inode_lba);

				break;
			}

			last_end_sector = partition_table[i].start_sector + partition_table[i].size;
		}



		updatePartionTable(partition_table, *drive);
	}
	else {

		print("SUPERBLOCK was found");


		drive->partitions[partition_index].meta_region = superblock->meta_region;
		drive->partitions[partition_index].data_region = superblock->data_region;
		drive->partitions[partition_index].start_lba = superblock->start_lba;
		drive->partitions[partition_index].sector_count = superblock->sector_count;
		drive->partitions[partition_index].root_inode_lba = superblock->root_inode_lba;

	}
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

int getSector(int sector_index, drive_t drive){

	int remainder = sector_index % 32;
	int* bmap = drive.sector_bitmap;

    int sector = (bmap[sector_index/32] >> remainder) & 0x1;
    
    return sector;
}	

int getFreeRegionExtent(int curr_sector, drive_t drive, int partition_index){

	int free_sectors = 1;
	int free_sector_index = curr_sector + 1;

	while(getSector(free_sector_index, drive) != 1 && free_sector_index < drive.partitions[partition_index].sector_count) {

		free_sectors++;
		free_sector_index++;
	}

	return free_sectors;
}

int findFirstFreeSector(drive_t drive, int partition_index){

	int curr_sector = drive.partitions[partition_index].meta_region; 

	while(true){

		if(getSector(curr_sector, drive) == 0){

			return curr_sector;
		}

		curr_sector += 1;
	}	
}

int findContinousRegion(int sectors_needed, drive_t drive, int partition_index) {

	int curr_ptr = drive.partitions[partition_index].data_region; // sector index tracker
	
	int best_fit_ptr = -1;
	int best_fit_free_region = -1;

	while(1) {

		// if the curr sector index is beyond physical capacity stop looking

		if(curr_ptr >= drive.partitions[partition_index].sector_count) {

			break;
		}
		else if(getSector(curr_ptr, drive) == 0){

			// when a free sector is found, try to see how far this free region extend
			
			int free_region = getFreeRegionExtent(curr_ptr, drive, partition_index);

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

void addEntries(char* data, inode_t* inode, int free_sectors, int starting_ptr, drive_t drive, int partition_index){

	//for initial inode
	
	#ifdef DEV 
		int num_entries = DEV;
	#else 
		int num_entries = (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t); 
	#endif
	
	int entry_table_sector = -1; // inode is already written to disk, no need to write it again

	entry_t* entries = inode->entries;

	while(true) { 

		for(int i = (entry_table_sector != -1 ? 1 : 0); i < num_entries - 1; i++) {

			if(entries[i].blocks == 0){

				print(" Extent entry found at ");
				printi(i);
				print(" ");

				entries[i].blocks = free_sectors;
				entries[i].start = starting_ptr;

				if(data != NULL) drive.write(free_sectors, starting_ptr, data, drive);

				setSectors(starting_ptr, starting_ptr + free_sectors, 1, drive);

				// update this entry table in disk
				if(entry_table_sector != -1) {

					drive.write(1, entry_table_sector, entries, drive);
				}

				//free(entries);

				return;
			}
		}

		print(" -> Going to next table ->");

		// if next entry table doesn't exist in last entry
		if(entries[num_entries - 1].blocks == 0) {

			// allocate new entry table
			entry_table_t* entry_table = (entry_table_t*)alloc_buffer(sizeof(entry_table_t), drive);

			// asign first entry in table
			entry_table->entries[0].blocks = free_sectors;
			entry_table->entries[0].start = starting_ptr;

			// find a free sector for it
			int new_entry_table_sector = findFirstFreeSector(drive, partition_index);


			println();
			print("Entry table allocated at sector ");
			printi(new_entry_table_sector);
			println();

			// write entry table to the sector 
			drive.write(1, new_entry_table_sector, entry_table, drive); 

			//set as used
			setSectors(new_entry_table_sector, new_entry_table_sector + 1, 1, drive);

			// assign last entry to new entryly table
			entries[num_entries - 1].blocks = 1;
			entries[num_entries -1].start = new_entry_table_sector;

			//update table in disk if its an extent table to point at tail
			if(entry_table_sector != -1){

				drive.write(1, entry_table_sector, entries, drive);
			}

			//free(entries);

			return;
		}
		else { 

		    entry_table_sector = entries[num_entries - 1].start;

			entries = (entry_t*)drive.read(1, entry_table_sector,  drive);
		}
		
		// update number of entries we have to look over 


		#ifndef DEV  
			num_entries = BLOCK_SIZE/sizeof(entry_t);
		#endif

	}

}




inode_t* createFile(char* data, inode_t* directory, char* file_name, int size, drive_t drive, int partition_index){

	// 1. Check if there's enough free sectors to store the file
	int avaliable_sectors = 0;
	int sectors_needed = ceil(size/512.0);

	/*
	for(int i = drive.partitions[partition_index].data_region; i <  drive.partitions[partition_index].sector_count; i++){

		if(getSector(i, drive) == 0){

			avaliable_sectors++;
		}
	}

	if(avaliable_sectors < sectors_needed) {

		//ERROR: not enough disk space

		print("Not enough disk space");

		return;
	}
	*/

	inode_t* inode = (inode_t*)alloc_buffer(sizeof(inode_t), drive);

	inode->meta.name = file_name;
	inode->meta.size = size;
	inode->meta.type = FILE;
	

	// 2. See if we can find a contious region of free space using best fit

	int continous_region_ptr = findContinousRegion(sectors_needed, drive, partition_index);

	if(continous_region_ptr == -1){

		// 3. A continous free region wasn't found. Use first fit to fill in fragments 

		int sectors_left = sectors_needed;
		int curr_ptr = drive.partitions[partition_index].data_region;
		int chunk_offset = 0;

		print("Couldn't find continous region");

		while(1){

			if(sectors_left <= 0){

				// 5. break once all sectors are accounted for

				break;
			}
			else if(getSector(curr_ptr, drive) == 0){

				// 4. get the extent of the free region once a free sector is found 

				int free_sectors = getFreeRegionExtent(curr_ptr, drive, partition_index);

				println();
				print(" Free sectors ");
				printi(free_sectors);

				// 5. Add entries to the inode

				char* split_data = (char*)alloc_buffer(512 * free_sectors, drive);
			
				for(int i = chunk_offset; i <  512 * free_sectors; i++) {

					split_data[i] = *( data + i );
				}

				addEntries(split_data, inode, free_sectors, curr_ptr, drive, partition_index);

				// 6. set inode blocks and increment/decrement

				sectors_left -= free_sectors;
				curr_ptr += free_sectors;
				chunk_offset += (free_sectors * 512);
			}
			else{

				curr_ptr += 1;
			}
		}
	}
	else {

		print("Found continous region");

		println();
		print(" Free sectors ");
		printi(sectors_needed);

		// 3. add entries 

		addEntries(data, inode, sectors_needed, continous_region_ptr, drive, partition_index);
	}

	//write inode to disk 

	int sector_for_inode = findFirstFreeSector(drive, partition_index);

	drive.write(1, sector_for_inode, inode, drive);

	println();
	print("Inode allocated at sector ");
	printi(sector_for_inode);
	println();

	setSectors(sector_for_inode, sector_for_inode + 1, 1, drive);

	// add to directory 

	addEntries(NULL, directory, 1, sector_for_inode, drive, partition_index);

	print("File created!");

	//free(data);
	//free(file_meta);

	return inode;

}


inode_t* directoryLookup(inode_t* inode, char* name, drive_t drive) {

	#ifdef DEV 
		int num_entries = DEV;
	#else 
		int num_entries = (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t); 
	#endif
	
	int entry_table_sector = -1; 

	entry_t* entries = inode->entries;

	while(true) { 

		for(int i = 0; i < num_entries - 1; i++) {
			
			inode_t* inode = (inode_t*)drive.read(entries[i].blocks, entries[i].start, drive);

			if(inode->meta.name == name) {

				return inode;
			}
		}

		if(entries[num_entries - 1].blocks != 0) {

		    entry_table_sector = entries[num_entries - 1].start;
			entries = (entry_t*)drive.read(1, entry_table_sector,  drive);
			
			#ifndef DEV
				num_entries = BLOCK_SIZE/sizeof(entry_t);
			#endif
		}
		else{

			return NULL;
		}
		
	}

	return NULL;
}

inode_t* resolvePathToInode(char* path) {

   drive_t* drive = NULL;
   inode_t* inode = NULL;

   char* folder = strok(path, '/'); 

   while(folder != NULL) {

   	  if(drive == NULL) {

   	  	 int drive_index = stringToNumber(folder);

   	  	 drive = &drives[drive_index];
   	  }
   	  else if(inode == NULL) {

   	  	 int partition_index = stringToNumber(folder);
   	  	 int root_lba = drive->partitions[partition_index].root_inode_lba;

   	  	 inode = (inode_t*)drive->read(1, root_lba, *drive);
   	  }
   	  else {

   	  	 inode = directoryLookup(inode, folder, *drive);
   	  }


	  folder = strok(NULL, '/');
   }
  
}

void openFile() {

	/*

		if doesn't exist, call create file and add to global file table

		else just add to global file table 

	*/
}

void writeFile() {

	/*	
		write file from current position 
	*/

}

void appendFile(){}
void renameFile(){}
void truncateFile(){}
void seekFile(){}
void closeFile(){}
void deleteFile(){}


void openDirectory(){}
void closeDirectory(){}
void deleteDirectory(){}



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
	
/*

	//leaving us with 7 free sectors 

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


	char* data = (char*)kalloc(512*10);
	memset(data, 512*10, 2);

	inode_t* directory = createDirectory(NULL, "root", drives[0], 1, -1);
	inode_t* inode = createFile(data, directory, "Hello", 512*10, drives[0], 1);

	char* data2 = (char*)kalloc(512);
	inode_t* inode2 = createFile(data2, directory, "Bye", 512, drives[0], 1);


	//clearScreen();
	//readFile(directory, drives[0]);

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
	

//	clearScreen();

//	readFile(inode, drives[0]);


	// Test 3. 2 entry tables 


	// Test 4. File too big

	/*
	for(int i = 0; i < 256; i++){

		printi(result[i]);

		print(" ");
	}
	*/


}
