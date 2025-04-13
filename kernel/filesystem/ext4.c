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
	 
	
	- child processes	
	- process busy handling
	- process file descriptors 
	- process signals (kill, interrupt)  
	- heap manangement per procss
	- stack management per process

	- finish LRU swapping

	- per process paging
		- cr3 context switch
		- demand paging

	- mouse interrupt and tracking
	- gui terminal 

	- PHASE 1: Complete , phase 2 will be the network stack

	- Subsequent phases:
		- More disk/ethernet drivers 
		- Full GUI support w/ multiple applications
		- Code compiler 
		- Eventually make 64 bit supported
*/


inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t* drive, int partition_index);


void cache(inode_t* inode, char* data, unsigned int start, unsigned int blocks) {

	if(inode == NULL) return;

	buffer_head_t* buffer_head = inode->buffer_head;

	if(buffer_head == NULL) {

		buffer_head = (buffer_head_t*)kalloc(sizeof(buffer_head_t));

		buffer_head->start = start;
		buffer_head->blocks = blocks;
		buffer_head->data = data;
		buffer_head->dirty = true;
		buffer_head->prev = NULL;
		buffer_head->next = NULL;

		inode->buffer_head = buffer_head;
	}
	else {

		buffer_head_t* new_buffer_head = (buffer_head_t*)kalloc(sizeof(buffer_head_t));
		new_buffer_head->start = start;
		new_buffer_head->blocks = blocks;
		new_buffer_head->dirty = true;
		new_buffer_head->data = data;
		new_buffer_head->prev = NULL;
		new_buffer_head->next = buffer_head;

		buffer_head->prev = new_buffer_head;

		inode->buffer_head = new_buffer_head;
	}
}

char* checkCache(inode_t* inode, unsigned int start) {

	if(inode == NULL) return NULL;

	buffer_head_t* buffer_head = inode->buffer_head;

	while(buffer_head->next != NULL) {

		if(buffer_head->start == start) {

			return buffer_head->data;
		}	

		buffer_head = buffer_head->next;
	}

	return NULL;
}

void initFs(drive_t* drive, int partition_index) {

	partition_entry_t* partition_table = readPartitionTable(*drive);
	superblock_t* superblock = readSuperBlock(partition_table, drive, partition_index);

	if(superblock == NULL){

		if(partition_table == NULL) { // for raw external drives 

			unsigned int data_region = 1 + (drive->sector_count * DATA_META_RATIO);

			updateSuperBlock(1, data_region, drive->sector_count, 0, drive, 0);
			createDirectory(NULL, "root", drive, 0);

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
				createDirectory(NULL, "root", drive, i);

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

void initEntryIterator(entry_iterator_t* it, inode_t* inode) {

	#ifdef DEV 
		it->num_entries = DEV;
	#else 
		it->num_entries = INODE_NUM_ENTRIES; 
	#endif

	it->i = 0;
	it->is_primary_table = true;
	it->entry_table_sector = inode->meta.lba;
	it->entries = inode->entries;
}




void entryIteratorAddEntry(entry_iterator_t* it, inode_t* inode, int blocks, int start, drive_t* drive, superblock_t* superblock) {
	
	while(true) { 

		while(it->i < it->num_entries - 1) {

			int i = it->i;

			it->i += 1;

			if(it->entries[i].blocks == 0){

				print(" Extent entry found at ");
				printi(i);
				print(" ");

				it->entries[i].blocks = blocks;
				it->entries[i].start = start;

				drive->write(1, it->entry_table_sector, it->is_primary_table ? inode : it->entries, *drive); 

				return;

			}
		}

		it->i = 0;

		// if next entry table doesn't exist in last entry
		if(it->entries[it->num_entries - 1].blocks == 0) {

			// allocate new entry table
			entry_table_t* entry_table = (entry_table_t*)allocBuffer(sizeof(entry_table_t), drive);

			// asign first entry in table
			entry_table->entries[0].blocks = blocks;
			entry_table->entries[0].start = start;

			// find a free sector for it
			int new_entry_table_sector = allocMetaSector(superblock);

			// write entry table to the sector 
			drive->write(1, new_entry_table_sector, entry_table, *drive); 

			// assign last entry to new entryly table
			it->entries[it->num_entries - 1].blocks = 1;
			it->entries[it->num_entries -1].start = new_entry_table_sector;

			//update table in disk if its an extent table to point at tail
			drive->write(1, it->entry_table_sector, it->is_primary_table ? inode : it->entries, *drive); 

			println();
			print("Entry table allocated at sector ");
			printi(new_entry_table_sector);
			println();

			return;
		}
		
		print(" -> Going to next table ->");

	    it->entry_table_sector = it->entries[it->num_entries - 1].start;
		it->entries = (entry_t*)drive->read(1, it->entry_table_sector, *drive);
		it->is_primary_table = false;

		#ifndef DEV  
		
			it->num_entries = EXT_TABLE_NUM_ENTRIES;
		
		#endif
	}
}

void addEntries(entry_iterator_t* it, inode_t* inode, int blocks, int start, drive_t* drive, superblock_t* superblock) {

	if(it == NULL) {

		entry_iterator_t it;

		initEntryIterator(&it, inode);
		entryIteratorAddEntry(&it, inode, blocks, start, drive, superblock);
	}
	else {

		entryIteratorAddEntry(it, inode, blocks, start, drive, superblock);
	}
}

inode_t* getNextEntry(entry_iterator_t* it, inode_t* inode, drive_t* drive) {

	while(true) { 

		while(it->i < it->num_entries - 1) {

			int start = it->entries[it->i].start;
			int blocks = it->entries[it->i].blocks;

			it->i += 1;

			if(blocks != 0) {

				inode_t* cached_inode = checkCache(inode, start);

				if(cached_inode != NULL) return cached_inode;

				return (inode_t*)drive->read(blocks, start, *drive);
			}
		}

		it->i = 0;

		if(it->entries[it->num_entries - 1].blocks != 0) {

		    it->entry_table_sector = it->entries[it->num_entries - 1].start;

		    it->entries = (entry_t*)checkCache(inode, it->entry_table_sector);
			
			if(it->entries == NULL) {

				it->entries = (entry_t*)drive->read(1, it->entry_table_sector, *drive);
			}

			#ifndef DEV
			
				it->num_entries = BLOCK_SIZE/sizeof(entry_t);
			
			#endif
		}
		else {

			return NULL;
		}
		
	}
}


inode_t* directoryLookup(inode_t* dir, char* name, drive_t* drive) {

	entry_iterator_t it;

	initEntryIterator(&it, dir);

	inode_t* i = getNextEntry(&it, dir, drive);

	while (i != NULL) {

		if(strcmp(i->meta.name, name)) {

			return i;
		}
		
		i = getNextEntry(&it, dir, drive);
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
   	  	
   	  	 *inode = (inode_t*)(*drive)->read(1, (*drive)->partitions[*partition_index].root_inode_lba, **drive);

   	  	 if(*inode == NULL) {

   	  	 	return false;
   	  	 }

   	  }
   	  else {

   	  	 inode_t* prev_inode = *inode;

   	  	 *inode = directoryLookup(*inode, *folder, *drive);

   	  	 if(*inode == NULL) {

   	  	 	*inode = prev_inode;

   	  	 	return false;
   	  	 }
   	  	 

   	  }

	  *folder = strok(NULL, '/');
   }

   return true;  
}


inode_t* createInode(inode_t* parent_directory, char* name, file_type type, drive_t* drive, int partition_index) {

	superblock_t* superblock = &drive->partitions[partition_index];
	
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

	drive->write(1, sector_for_inode, inode, *drive);

	if(parent_directory) addEntries(NULL, parent_directory, 1, sector_for_inode, drive, superblock);

	return inode;
}

inode_t* createDirectory(inode_t* parent_directory, char* name, drive_t* drive, int partition_index) {

	return createInode(parent_directory, name, DIR, drive, partition_index);
}


inode_t* createFile(inode_t* parent_directory, char* name, drive_t* drive, int partition_index) {

	return createInode(parent_directory, name, FILE, drive, partition_index);		
}

int open(char* path) {
	
	inode_t* inode = NULL; drive_t* drive = NULL; int partition_index; char* file_name;
	
	bool file_found = resolvePath(path, &inode, &drive, &partition_index, &file_name);

	if(!file_found) {

		inode = createFile(inode, file_name, drive, partition_index);
	}

	if(inode == NULL) return -1;


	// add inode and each ext entry to cache 

	
	int fd = fd_acc;
	int fd_code = addToFileTable(inode, fd_acc, drive, partition_index);

	//kfree(inode);

	if(fd_code != -1) {

		fd_acc += 1;

		return fd;
	}
	else { 

		return -1;
	}


}

char* splitData(char* data, int chunk_offset, int free_sectors, drive_t* drive){

	char* split_data = (char*)allocBuffer(BLOCK_SIZE * free_sectors, drive);
    
    for(int i = 0; i < BLOCK_SIZE * free_sectors; i++) {
       
        split_data[i] = *(data + chunk_offset + i);
    }

    return split_data;
}


void append(entry_iterator_t* it, inode_t* inode, int blocks_left, char* data, file_table_entry_t* ft_entry, drive_t* drive, superblock_t* superblock) {

	int* sector_bitmap = superblock->sector_bitmap;
	int data_region = superblock->data_region;
	int sector_count = superblock->sector_count;
    int continuous_region_start = findContinousFreeRegion(blocks_left, data_region, sector_count, sector_bitmap);

    if (continuous_region_start == -1) {

        print("Couldn't find continuous region! \n");

        int curr_sector = data_region;
        int data_offset = 0;

        while (blocks_left > 0) {

            if (getSector(curr_sector, superblock->sector_bitmap) == 0) {

                int free_sectors = getFreeRegionExtent(curr_sector, superblock->sector_count, superblock->sector_bitmap);
                
                free_sectors = (free_sectors > blocks_left) ? blocks_left : free_sectors;
                
                char* split_data = splitData(data, data_offset, free_sectors, drive);

                drive->write(free_sectors, curr_sector, split_data, *drive);
                
                cache(inode, data, curr_sector, free_sectors);
                addEntries(it, inode, free_sectors, curr_sector, drive, superblock);
                setSectors(curr_sector, curr_sector + free_sectors, 1, superblock->sector_bitmap);

                int bytes_written = free_sectors * BLOCK_SIZE;

                blocks_left -= free_sectors;
                data_offset += bytes_written;
                curr_sector += free_sectors;
                ft_entry->fpos += bytes_written;
                inode->meta.size += bytes_written;

            } 
            else {

                curr_sector += 1;
            }
        }

    } else {

        char* split_data = splitData(data, 0, blocks_left, drive);

        drive->write(blocks_left, continuous_region_start, split_data, *drive);
       
        cache(inode, data, continuous_region_start, blocks_left);
        addEntries(it, inode, blocks_left, continuous_region_start, drive, superblock);
        setSectors(continuous_region_start, continuous_region_start + blocks_left, 1, superblock->sector_bitmap);

        int bytes_written = blocks_left * BLOCK_SIZE;
        
        ft_entry->fpos += bytes_written;
        inode->meta.size += bytes_written;
    }
}


void write(unsigned int fd, char* data, int size) {

    file_table_entry_t* ft_entry = getFileTableEntry(fd);
    
    if (ft_entry == NULL) return;

    drive_t* drive = ft_entry->drive;
    inode_t* inode = (inode_t*)ft_entry->inode;
    superblock_t* superblock = ft_entry->superblock;
    
    if(inode == NULL || drive == NULL || superblock == NULL) return;

    entry_t* ext_entry;
    entry_iterator_t get_it;
    entry_iterator_t add_it; 

    initEntryIterator(&get_it, inode);
    initEntryIterator(&add_it, inode);

    int byte_offset = 0;
    int data_offset = 0;
    int blocks_left = ceil(size / (double)BLOCK_SIZE);

    while ((ext_entry = getNextEntry(&get_it, inode, drive)) != NULL && blocks_left > 0) {
    	
    	int ext_entry_size = (ext_entry->blocks * BLOCK_SIZE);

        if (ft_entry->fpos >= byte_offset && ft_entry->fpos < byte_offset + ext_entry_size) {

            int copy_block_start = (ft_entry->fpos - byte_offset) / BLOCK_SIZE;
            int copy_block_sector = ext_entry->start + copy_block_start;
            int copy_block_end = min(blocks_left - copy_block_start, ext_entry->blocks - copy_block_start);
            int total_copy_blocks = copy_block_end - copy_block_start;
            int total_after_blocks = max(ext_entry->blocks - (copy_block_start + total_copy_blocks), 0);
            int total_bytes_copied = total_copy_blocks * BLOCK_SIZE;

            if (copy_block_start != 0) {

                ext_entry->blocks -= (total_copy_blocks + total_after_blocks);
                
                addEntries(NULL, inode, total_copy_blocks, copy_block_sector, drive, superblock);

            } else {

                ext_entry->blocks = total_copy_blocks;
            }

            char* split_data = splitData(data, data_offset, total_copy_blocks, drive);
            
            drive->write(total_copy_blocks, copy_block_sector, split_data, *drive);

            if (total_after_blocks > 0) {
               
                addEntries(NULL, inode, total_after_blocks, copy_block_sector + total_copy_blocks, drive, superblock);
            }

            blocks_left -= total_copy_blocks;
            data_offset += total_bytes_copied;

            //advance fpos
            ft_entry->fpos += total_bytes_copied;

        }

        byte_offset += ext_entry_size;
    }

    if (blocks_left > 0) {

        print("BLOCKS LEFT (appending): ");
        printi(blocks_left);
        print("\n");

        append(&add_it, inode, blocks_left, data + data_offset, ft_entry, drive, superblock);
    }
}


void read(int fd, char* buffer){
	
	file_table_entry_t* ft_entry = getFileTableEntry(fd);
	
	if(ft_entry == NULL) return;

	inode_t* inode = (inode_t*)ft_entry->inode;
	drive_t* drive = ft_entry->drive;
	superblock_t* superblock = ft_entry->superblock;
	
	if(inode == NULL || drive == NULL || superblock == NULL) return;

	entry_t* ext_entry;
	entry_iterator_t it;

	initEntryIterator(&it, inode);

	int data_offset = 0;

	while ((ext_entry = getNextEntry(&it, inode, drive)) != NULL) {

		
		char* data = checkCache(inode, ext_entry->start);

		if(data == NULL) {
	
			data = (char*)drive->read(ext_entry->blocks, ext_entry->start, *drive);	
		}

		for(int j = 0; j < ext_entry->blocks * BLOCK_SIZE; j++){

			buffer[data_offset + j] = data[j];
		}

		data_offset += ext_entry->blocks * BLOCK_SIZE;
		
	}
}



void rename(){}
void close(){}
void delete(){}


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

	inode_t* parent_directory = (inode_t*)drives[0].read(1, drives[0].partitions[1].root_inode_lba, drives[0]);
	inode_t* directory = createDirectory(parent_directory, "thing1", &drives[0], 1);
	inode_t* directory2 = createDirectory(directory, "thing2", &drives[0], 1);

	//createFile(directory2, "Bye", drives[0], 1);
	//createFile(directory2, "Hello", drives[0], 1);

/*
	inode_t* test = directoryLookup(directory2, "Bye", &drives[0]);

	print("Name: ");
	print(test->meta.name);
*/

	int fd = open("0/1/thing1/thing2/Bye");
	
	/*
	int fd2 = open("0/1/thing1/thing2/Hello");
	int fd3 = open("0/1/thing1/thing2/Good");
	int fd4 = open("0/1/thing1/thing2/Boy");
	int fd5 = open("0/1/thing1/thing2/Must");
	int fd6 = open("0/1/thing1/thing2/Love");
	int fd7 = open("0/1/thing1/thing2/Now");

		*/
	
	//write(fd, data, 512*10);
	//read(fd, buffer);

	/*clearScreen();

	for(int i = 0; i < 512*10; i++) {

		printi(buffer[i]);
	}*/



	/*
	inode_t* inode_3 = (inode_t*)file_table[0].inode;
	inode_t* inode_4 = (inode_t*)file_table[0].next->inode;
	inode_t* inode_5 = (inode_t*)file_table[0].next->next->inode;
	*/

	
	file_table_entry_t* e1 = getFileTableEntry(fd);
	inode_t* inode_3 = (inode_t*)e1->inode;
	/*
	file_table_entry_t* e2 = getFileTableEntry(fd2);
	file_table_entry_t* e3 = getFileTableEntry(fd3);

	inode_t* inode_3 = (inode_t*)e1->inode;
	inode_t* inode_4 = (inode_t*)e2->inode;
	
	inode_t* inode_5 = (inode_t*)e3->inode;
	

	//write(fd, data, 512*10);

	
	println();
	print(inode_3->meta.name);

	println();
	print(inode_4->meta.name);

	println();
	print(inode_5->meta.name);
	*/

	
	write(fd, data, 512*10);

	read(fd, buffer);

	clearScreen();

	for(int i = 0; i < 512*10; i++){

		printi(buffer[i]);
	}
	
	
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
