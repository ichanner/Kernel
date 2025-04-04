#include <stdbool.h>
#include <stddef.h>
#include "fs.h"
#include "math.c"
#include "./partition.c"

/*
	TODO:
	

	
	- filesystem
	
	- finish LRU swapping

	- per process paging
		- cr3 context switch
		- demand paging 
	
	- child processes
	- cpu threads per process
	- process busy handling
	- send process signals (kill, interrupt, pause etc)  
	- heap manangement per procss
	- stack management per process

	- mouse interrupt and tracking
	- gui terminal 

	- PHASE 1: Complete , phase 2 will be the network stack

	- Subsequent phases:
		- More disk/ethernet drivers 
		- Full GUI support w/ multiple applications
		- Code compiler 
		- Eventually make 64 bit supported
*/


void* allocBuffer(unsigned int size, drive_t drive) {

	if(drive.dma_mode != -1) {

		void* buffer = allocPages(size);

		identityMapPages(buffer, size, 1, 0, 0, 1);

		return buffer;
	}

	return kalloc(size);
}

inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t drive, int partition_index, bool is_root) {

	inode_t* inode = (inode_t*)allocBuffer(sizeof(inode_t), drive);
	const int sector_for_dir = allocMetaSector(drive, partition_index);

	inode->meta.type = DIR;
	inode->meta.name = name;
	inode->meta.size = 0;
	inode->meta.lba = sector_for_dir;

	drive.write(1, sector_for_dir, inode, drive);

	if(parent_directory != NULL) {

		addEntries(parent_directory, 1, sector_for_dir, drive, partition_index);
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
			createDirectory(NULL, "root", *drive, 0, true);

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
				createDirectory(NULL, "root", *drive, i, true);

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

		int* sector_bitmap = (int*)drive->read(superblock->bitmap_sector_count, superblock->bitmap_start_sector, *drive);
		
		drive->partitions[partition_index].sector_bitmap = sector_bitmap;

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


int allocMetaSector(drive_t drive, int partition_index){

	superblock_t* superblock = &drive.partitions[partition_index];
	if(superblock == NULL) return -1;

	int curr_ptr = superblock->meta_region; 

	while(curr_ptr < superblock->data_region){

		if(getSector(curr_ptr, drive) == 0) {

			setSectors(curr_ptr, curr_ptr + 1, 1, drive);

			return curr_ptr;
		}

		curr_ptr += 1;
	}	

	return -1;
}

int getFreeRegionExtent(int curr_ptr, const int limit, drive_t drive) {

	int free_sectors = 1;
	int free_sector_index = curr_ptr + 1;

	while(getSector(free_sector_index, drive) != 1 && free_sector_index < limit) {

		free_sectors++;
		free_sector_index++;
	}

	return free_sectors;
}

int findContinousFreeRegion(int sectors_needed, int start_ptr, const int limit, drive_t drive) {

	int curr_ptr = start_ptr; // sector index tracker
	int best_fit_ptr = -1;
	int best_fit_free_region = -1;

	// if the curr sector index is beyond physical capacity stop looking
	while(curr_ptr < limit) { 	

		if(getSector(curr_ptr, drive) == 0){

			// when a free sector is found, try to see how far this free region extend
			int free_region = getFreeRegionExtent(curr_ptr, limit, drive);

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

void addEntries(inode_t* inode, int free_sectors, int starting_ptr, drive_t drive, int partition_index){

	//for initial inode
	
	#ifdef DEV 
		int num_entries = DEV;
	#else 
		int num_entries = (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t); 
	#endif
	
	//int entry_table_sector = -1; // inode is already written to disk, no need to write it again

	bool is_inode = true;
	int entry_table_sector = inode->meta.lba;

	entry_t* entries = inode->entries;

	while(true) { 

		for(int i = 0; i < num_entries - 1; i++) {

			if(entries[i].blocks == 0){

				print(" Extent entry found at ");
				printi(i);
				print(" ");

				entries[i].blocks = free_sectors;
				entries[i].start = starting_ptr;

				drive.write(1, entry_table_sector, is_inode ? inode : entries, drive); 
		
				//free(entries);

				return;
			}
		}

		print(" -> Going to next table ->");

		// if next entry table doesn't exist in last entry
		if(entries[num_entries - 1].blocks == 0) {

			// allocate new entry table
			entry_table_t* entry_table = (entry_table_t*)allocBuffer(sizeof(entry_table_t), drive);

			// asign first entry in table
			entry_table->entries[0].blocks = free_sectors;
			entry_table->entries[0].start = starting_ptr;

			// find a free sector for it
			int new_entry_table_sector = allocMetaSector(drive, partition_index);

			println();
			print("Entry table allocated at sector ");
			printi(new_entry_table_sector);
			println();

			// write entry table to the sector 
			drive.write(1, new_entry_table_sector, entry_table, drive); 


			// assign last entry to new entryly table
			entries[num_entries - 1].blocks = 1;
			entries[num_entries -1].start = new_entry_table_sector;

			//update table in disk if its an extent table to point at tail
			drive.write(1, entry_table_sector, is_inode ? inode : entries, drive); 

			//free(entries);

			return;
		}
		else { 

		    entry_table_sector = entries[num_entries - 1].start;
			entries = (entry_t*)drive.read(1, entry_table_sector,  drive);
			is_inode = false;
		}
		
		// update number of entries we have to look over 


		#ifndef DEV  
			num_entries = BLOCK_SIZE/sizeof(entry_t);
		#endif

	}

}


inode_t* createFile(char* data, inode_t* directory, char* file_name, int size, drive_t drive, int partition_index){

	superblock_t* superblock = &drive.partitions[partition_index];
	if(superblock == NULL) return -1;

	const int sectors_needed = ceil(size/512.0);
	const int sector_for_inode = allocMetaSector(drive, partition_index);
	const int continous_region_ptr = findContinousFreeRegion(sectors_needed, superblock->data_region, superblock->sector_count, drive);

	inode_t* inode = (inode_t*)allocBuffer(sizeof(inode_t), drive);

	inode->meta.name = file_name;
	inode->meta.size = size;
	inode->meta.type = FILE;
	inode->meta.lba = sector_for_inode;

	if(continous_region_ptr == -1) {

		int sectors_left = sectors_needed;
		int curr_ptr = superblock->data_region;
		int chunk_offset = 0;

		print("Couldn't find continous region");

		while(sectors_left > 0){

			if(getSector(curr_ptr, drive) == 0){

				// 4. get the extent of the free region once a free sector is found 
				int free_sectors = getFreeRegionExtent(curr_ptr, superblock->sector_count, drive);

				println();
				print(" Free sectors ");
				printi(free_sectors);

				// 5. Add entries to the inode

				char* split_data = (char*)allocBuffer(512 * free_sectors, drive);
			
				for(int i = chunk_offset; i <  512 * free_sectors; i++) {

					split_data[i] = *( data + i );
				}

				addEntries(inode, free_sectors, curr_ptr, drive, partition_index);

				drive.write(free_sectors, curr_ptr, split_data, drive);
				setSectors(curr_ptr, curr_ptr + free_sectors, 1, drive);

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

		addEntries(inode, sectors_needed, continous_region_ptr, drive, partition_index);

		drive.write(sectors_needed, continous_region_ptr, data, drive);
		setSectors(continous_region_ptr, continous_region_ptr + sectors_needed, 1, drive);
	}

	println();
	print("Inode allocated at sector ");
	printi(sector_for_inode);
	println();

	// add to directory 
	
	addEntries(directory, 1, sector_for_inode, drive, partition_index);

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

	
			if(strcmp(inode->meta.name, name)) {

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

bool resolvePathToInode(char* path, inode_t** inode) {

   drive_t* drive = NULL;

   char* folder = strok(path, '/'); 

   while(folder != NULL) {

   	  if(drive == NULL) {

   	  	 int drive_index = stringToNumber(folder);

   	  	 drive = &drives[drive_index];
   	  }
   	  else if(*inode == NULL) {

   	  	 int partition_index = stringToNumber(folder);
   	  	 int root_lba = drive->partitions[partition_index].meta_region;

   	  	 *inode = (inode_t*)drive->read(1, root_lba, *drive);

   	  	 if(*inode == NULL) {

   	  	 	return false;
   	  	 }

   	  }
   	  else {

   	  	 inode_t* prev_inode = *inode;
   	  	 
   	  	 *inode = directoryLookup(*inode, folder, *drive);

   	  	 if(*inode == NULL) {

   	  	 	*inode = prev_inode;

   	  	 	return false;
   	  	 }

   	  }

	  folder = strok(NULL, '/');
   }

   return true;  
}

inode_t* createFile_v2(inode_t* parent_directory, char* filename, drive_t drive, int partition_index) {

	/*
	inode_t* new_file = (inode_t*)kalloc(sizeof(inode_t));

	int sector_for_file = allocMetaSector(drive, partition_index);

	new_file->meta.name = filename;
	new_file->meta.name = sizeof(inode_t);
	new_file->meta.lba = sector_for_file;

	// update directory 

	if(parent_directory != NULL) {

		addEntries()
	}	
*/

}

void openFile(char* path) {
/*
	bool file_found; 
	
	inode_t* inode = resolvePathToInode(path, &file_found, );

	if(!file_found) {

		inode = createFile_v2(inode, );
	}
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
void readFile();
void closeFile(){}
void deleteFile(){}
void readDirectory(){}
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
	char* data2 = (char*)kalloc(512);

	memset(data, 512*10, 2);

	//inode_t* parent = (inode_t*)drives[0].read(1, drives[0].partitions[1].root_inode_lba, drives[0]);

	inode_t* parent_directory = (inode_t*)drives[0].read(1, drives[0].partitions[1].meta_region, drives[0]);
	inode_t* directory = createDirectory(parent_directory, "thing1", drives[0], 1, false);
	inode_t* directory2 = createDirectory(directory, "thing2", drives[0], 1, false);

	inode_t* inode = createFile(data, directory2, "Hello", 512*10, drives[0], 1);
	inode_t* inode2 = createFile(data2, directory2, "Bye", 512, drives[0], 1);

	inode_t* inode3 = NULL;

	bool file_found = resolvePathToInode("0/1/thing1/thing2/Bye", &inode3);

	if(file_found) {

		println();
		print(inode3->meta.name);

	}
	else {

		print("Nothing: ");

		println();
		print(inode3->meta.name);
	}

	/*
	
	inode_t* directory2 = createDirectory(directory, "thing2", drives[0], 1, false);

	inode_t* inode = createFile(data, directory2, "Hello", 512*10, drives[0], 1);

	char* data2 = (char*)kalloc(512);
	inode_t* inode2 = createFile(data2, directory2, "Bye", 512, drives[0], 1);



	inode_t* inode3 = resolvePathToInode("0/1/thing1/thing2");

	println();
	print(inode3->meta.name);
	*/


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
