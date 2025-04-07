#include <stdbool.h>
#include <stddef.h>
#include "ext4.h"
#include "./bmap.c"
#include "./allocator.c"
#include "./ftable.c"
#include "../utils/math.c"
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


inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t drive, int partition_index);

void initFs(drive_t* drive, int partition_index) {

	partition_entry_t* partition_table = readPartitionTable(*drive);
	superblock_t* superblock = readSuperBlock(partition_table, drive, partition_index);

	if(superblock == NULL){

		if(partition_table == NULL) { // for raw external drives 

			unsigned int data_region = 1 + (drive->sector_count * DATA_META_RATIO);

			updateSuperBlock(1, data_region, drive->sector_count, 0, drive, 0);
			createDirectory(NULL, "root", *drive, 0);

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

				unsigned int meta_region = partition_table[i].start_sector;
				unsigned int data_region = partition_table[i].start_sector + (partition_table[i].size * DATA_META_RATIO);
				unsigned int sector_count = partition_table[i].size;
				unsigned int start_lba = partition_table[i].start_sector;
	
				updateSuperBlock(meta_region, data_region, sector_count, start_lba, drive, partition_index);
				
				createDirectory(NULL, "root", *drive, i);

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
		drive->partitions[partition_index].bitmap_sector_count = superblock->bitmap_sector_count;
		drive->partitions[partition_index].bitmap_start_sector = superblock->bitmap_start_sector;

		int* sector_bitmap = (int*)drive->read(superblock->bitmap_sector_count, superblock->bitmap_start_sector, *drive);
		
		drive->partitions[partition_index].sector_bitmap = sector_bitmap;
	}
}


void addEntries(inode_t* inode, int free_sectors, int starting_ptr, drive_t drive, superblock_t* superblock, add_entry_state_t* prev_state) {
	
	int num_entries; int first_index; bool is_primary_table; int entry_table_sector; entry_t* entries; 

	#ifdef DEV 
		num_entries = DEV;
	#else 
		num_entries = INODE_NUM_ENTRIES; 
	#endif

	if(prev_state == NULL) {

		is_primary_table = true;
		first_index = 0;
		entry_table_sector = inode->meta.lba;
		entries = inode->entries;
	}
	else {

		is_primary_table = prev_state->is_primary_table;
		entry_table_sector = prev_state->entry_table_sector;
		entries = prev_state->entries; 
		first_index = prev_state->first_index;
		num_entries = prev_state->num_entries;
	}

	while(true) { 

		for(int i = first_index; i < num_entries - 1; i++) {

			if(entries[i].blocks == 0){

				print(" Extent entry found at ");
				printi(i);
				print(" ");

				entries[i].blocks = free_sectors;
				entries[i].start = starting_ptr;

				drive.write(1, entry_table_sector, is_primary_table ? inode : entries, drive); 

				prev_state->entries = entries;
				prev_state->entry_table_sector = entry_table_sector;
				prev_state->is_primary_table = is_primary_table;
				prev_state->num_entries = num_entries;
				prev_state->first_index = i + 1;

				return;

			}
		}

		// if next entry table doesn't exist in last entry
		if(entries[num_entries - 1].blocks == 0) {

			// allocate new entry table
			entry_table_t* entry_table = (entry_table_t*)allocBuffer(sizeof(entry_table_t), drive);

			// asign first entry in table
			entry_table->entries[0].blocks = free_sectors;
			entry_table->entries[0].start = starting_ptr;

			// find a free sector for it
			int new_entry_table_sector = allocMetaSector(superblock);

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
			drive.write(1, entry_table_sector, is_primary_table ? inode : entries, drive); 

			prev_state->entries = entry_table->entries;
			prev_state->entry_table_sector = new_entry_table_sector;
			prev_state->is_primary_table = false;
			prev_state->first_index = 1;

			#ifdef DEV 
				prev_state->num_entries = DEV;
			#else 
				prev_state->num_entries = EXT_TABLE_NUM_ENTRIES;
			#endif

			return;
		}
		else { 

			print(" -> Going to next table ->");

		    entry_table_sector = entries[num_entries - 1].start;
			entries = (entry_t*)drive.read(1, entry_table_sector,  drive);
		}
		

		is_primary_table = false;

		#ifndef DEV  
		
			num_entries = EXT_TABLE_NUM_ENTRIES;
		
		#endif

	}
}

inode_t* directoryLookup(inode_t* inode, char* name, drive_t drive) {

	#ifdef DEV 
		int num_entries = DEV;
	#else 
		int num_entries = INODE_NUM_ENTRIES; 
	#endif
	
	entry_t* entries = inode->entries;

	while(true) { 

		for(int i = 0; i < num_entries - 1; i++) {

			if(entries[i].blocks != 0){
			
				inode_t* inode = (inode_t*)drive.read(entries[i].blocks, entries[i].start, drive);	
		
				if(strcmp(inode->meta.name, name)) {

					return inode;
				}

			}
		}

		if(entries[num_entries - 1].blocks != 0) {

		    int entry_table_sector = entries[num_entries - 1].start;
			
			entries = (entry_t*)drive.read(1, entry_table_sector,  drive);
			
			#ifndef DEV
			
				num_entries = EXT_TABLE_NUM_ENTRIES;
			
			#endif
		}
		else{

			return NULL;
		}
		
	}

	return NULL;
}

bool resolvePath(char* path, inode_t** inode, drive_t** drive, int* partition_index, char** folder) {

   *folder = strok(path, '/'); 

   while(*folder != NULL) {

   	  if(*drive == NULL) {

   	  	 int drive_index = stringToNumber(*folder);

   	  	 *drive = &drives[drive_index];
   	  }
   	  else if(*inode == NULL) {

   	  	 *partition_index = stringToNumber(*folder);
   	  	
   	  	 *inode = (inode_t*)(*drive)->read(1, (*drive)->partitions[*partition_index].meta_region, **drive);

   	  	 if(*inode == NULL) {

   	  	 	return false;
   	  	 }

   	  }
   	  else {

   	  	 inode_t* prev_inode = *inode;

   	  	 *inode = directoryLookup(*inode, *folder, **drive);

   	  	 if(*inode == NULL) {

   	  	 	*inode = prev_inode;

   	  	 	return false;
   	  	 }

   	  }

	  *folder = strok(NULL, '/');
   }

   return true;  
}


inode_t* createInode(inode_t* parent_directory, char* name, file_type type, drive_t drive, int partition_index) {

	superblock_t* superblock = &drive.partitions[partition_index];
	
	const int sector_for_inode = allocMetaSector(superblock);

	if(sector_for_inode == -1) return NULL;

	inode_t* inode = (inode_t*)allocBuffer(sizeof(inode_t), drive);

	if(inode == NULL) return NULL;

	inode->meta.name = name;
	inode->meta.size = sizeof(inode_t);
	inode->meta.type = type;
	inode->meta.lba = sector_for_inode;

	println();
	print("Sector for inode: ");
	printi(sector_for_inode);

	drive.write(1, sector_for_inode, inode, drive);

	if(parent_directory) addEntries(parent_directory, 1, sector_for_inode, drive, superblock, NULL);

	return inode;
}

inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t drive, int partition_index) {

	return createInode(parent_directory, name, DIR, drive, partition_index);
}


inode_t* createFile_v2(inode_t* parent_directory, char* name, drive_t drive, int partition_index) {

	return createInode(parent_directory, name, FILE, drive, partition_index);		
}

int openFile(char* path) {
	
	inode_t* inode = NULL; drive_t* drive = NULL; int partition_index; char* file_name;
	
	bool file_found = resolvePath(path, &inode, &drive, &partition_index, &file_name);

	if(!file_found) {

		print("name: ");
		print(file_name);

		inode = createFile_v2(inode, file_name, *drive, partition_index);
	}

	if(inode == NULL) return -1;

	int fd = fd_acc;
	int fd_code = addToFileTable(inode, fd_acc, *drive, partition_index);

	//kfree(inode);

	if(fd_code != -1) {

		fd_acc += 1;

		return fd;
	}
	else { 

		return -1;
	}


}


void writeFile(unsigned int fd, char* data, int size) {

	file_table_entry_t* entry = getFileTableEntry(fd);

	if(entry == NULL) return;

	superblock_t superblock = entry->superblock;
	drive_t drive = entry->drive;
	inode_t* inode = (inode_t*)entry->inode;

	const int sectors_needed = ceil(size/512.0);
	const int continous_region_start = findContinousFreeRegion(sectors_needed, superblock.data_region, superblock.sector_count, superblock.sector_bitmap);

	if(continous_region_start == -1) {

		print("Couldn't find continous region! \n");

		int sectors_left = sectors_needed;
		int curr_ptr = superblock.data_region;
		int chunk_offset = 0;

		add_entry_state_t* curr_state = (add_entry_state_t*)kalloc(sizeof(add_entry_state_t));

		curr_state->is_primary_table = true;
		curr_state->first_index = 0;
		curr_state->entry_table_sector = inode->meta.lba;
		curr_state->entries = inode->entries;

		#ifdef DEV
			curr_state->num_entries = DEV;
		#else 
			curr_state->num_entries = INODE_NUM_ENTRIES;
		#endif
		
		while(sectors_left > 0){

			if(getSector(curr_ptr, superblock.sector_bitmap) == 0){

				int free_sectors = getFreeRegionExtent(curr_ptr, superblock.sector_count, superblock.sector_bitmap);

				print("Free sectors found: ");
				printi(free_sectors);

				char* split_data = (char*)allocBuffer(512 * free_sectors, drive);
			
				for(int i = chunk_offset; i <  512 * free_sectors; i++) {

					split_data[i] = *( data + i );
				}

				drive.write(free_sectors, curr_ptr, split_data, drive);

				addEntries(inode, free_sectors, curr_ptr, drive, &superblock, curr_state);
				setSectors(curr_ptr, curr_ptr + free_sectors, 1, superblock.sector_bitmap);

				println();
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

		print("Continous region found! \n");

		drive.write(sectors_needed, continous_region_start, data, drive);

		addEntries(entry->inode, sectors_needed, continous_region_start, drive, &superblock, NULL);
		setSectors(continous_region_start, continous_region_start + sectors_needed, 1, superblock.sector_bitmap);
	}
}

void readFile(int fd, char* buffer){

	#ifdef DEV 
		int num_entries = DEV;
	#else 
		int num_entries = (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t); 
	#endif
	
	file_table_entry_t* entry = getFileTableEntry(fd);
	inode_t* inode = (inode_t*)entry->inode;
	drive_t drive = entry->drive;
	entry_t* entries = inode->entries;

	int chunk_offset = 0;

	while(true) { 

		for(int i = 0; i < num_entries - 1; i++) {

			if(entries[i].blocks != 0){
			
				char* data = (char*)drive.read(entries[i].blocks, entries[i].start, drive);	

				for(int j = 0; j < entries[i].blocks * 512; j++){

					buffer[chunk_offset + j] = data[j];
				}

				chunk_offset += entries[i].blocks * 512;
		
			}
		}

		if(entries[num_entries - 1].blocks != 0) {

		    int entry_table_sector = entries[num_entries - 1].start;
			
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

void appendFile(int fd, char* data, int size){


}

void renameFile(){


}
void seekFile(){}

void closeFile(){}
void deleteFile(){}
void readDirectory(){}
void openDirectory(){}
void closeDirectory(){}
void deleteDirectory(){}



void test_fs(){


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

	

	/*
	setSectors(60, 100, 1,  drives[0].partitions[1].sector_bitmap);
	setSectors(60, 61, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(65, 66, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(70, 71, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(75, 77, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(80, 81, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(85, 86, 0,  drives[0].partitions[1].sector_bitmap);
	setSectors(90, 91, 0, drives[0].partitions[1].sector_bitmap);
	setSectors(95, 96, 0, drives[0].partitions[1].sector_bitmap);
	*/
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

	print("Meta Bitmap: ");

	for(int i = drives[0].partitions[1].meta_region; i < drives[0].partitions[1].data_region; i++){

		printi(getSector(i, drives[0].partitions[1].sector_bitmap));
		print(" ");
	}


	char* data = (char*)kalloc(512*10);
	char* buffer = (char*)kalloc(512*10);

	memset(data, 512*10, 2);

	//inode_t* parent = (inode_t*)drives[0].read(1, drives[0].partitions[1].root_inode_lba, drives[0]);

	inode_t* parent_directory = (inode_t*)drives[0].read(1, drives[0].partitions[1].meta_region, drives[0]);
	inode_t* directory = createDirectory(parent_directory, "thing1", drives[0], 1);
	inode_t* directory2 = createDirectory(directory, "thing2", drives[0], 1);

	//createFile_v2(directory2, "Hello", drives[0], 1);

	
	int fd = openFile("0/1/thing1/thing2/Bye");
	int fd2 = openFile("0/1/thing1/thing2/Hello");
	int fd3 = openFile("0/1/thing1/thing2/Good");
	clearScreen();

	writeFile(fd, data, 512*10);

	readFile(fd, buffer);


	for(int i = 0; i < 512*2; i++){

		printi(buffer[i]);
	}

	/*
	inode_t* inode_3 = (inode_t*)file_table[0].inode;
	inode_t* inode_4 = (inode_t*)file_table[0].next->inode;
	inode_t* inode_5 = (inode_t*)file_table[0].next->next->inode;
	*/

	file_table_entry_t* e1 = getFileTableEntry(fd);
	file_table_entry_t* e2 = getFileTableEntry(fd2);
	file_table_entry_t* e3 = getFileTableEntry(fd3);

	inode_t* inode_3 = (inode_t*)e1->inode;
	inode_t* inode_4 = (inode_t*)e2->inode;
	inode_t* inode_5 = (inode_t*)e3->inode;

	println();
	print(inode_3->meta.name);

	println();
	print(inode_4->meta.name);

	println();
	print(inode_5->meta.name);

	/*
	inode_t* inode3 = NULL;
	drive_t* drive = NULL;
	int partition_index;
	char* name;
	bool file_found = resolvePath("0/1/thing1/thing2/Bye", &inode3, &drive, &partition_index, &name);

	if(file_found) {

		println();
		print(inode3->meta.name);
	

	}
	else {

		print("Nothing: ");

		println();
		print(inode3->meta.name);

	}

	*/

	syncBitmap(drives[0], 1);



	//drives[0].write(drives[0].partitions[1].bitmap_sector_count, drives[0].partitions[1].bitmap_start_sector, drives[0].partitions[1].sector_bitmap, drives[0]);

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

	for(int i = drives[0].partitions[1].meta_region; i < drives[0].partitions[1].data_region; i++){

		printi(getSector(i, drives[0].partitions[1].sector_bitmap));
		print(" ");
	}
	*/
/*
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
