#include "ftable.h"

int fd_acc = 0;
int file_table_size = 10;

file_table_entry_t* file_table;

void initFileTable() { 

	file_table = (file_table_entry_t*)kalloc(sizeof(file_table_entry_t) * file_table_size);
}

int hash(int key) {

	return key % file_table_size;
}


file_table_entry_t* getFileTableEntry(int fd) {

	int index = hash(fd);

	file_table_entry_t* entry = (file_table_entry_t*)(file_table + index);

	while(1) {

		if (entry == NULL) break;

		if(entry->fd == fd) {

			return entry;
		}

		entry = entry->next;
	}

	return NULL;

}

int addToFileTable(void* inode, int fd, drive_t drive, int partition_index) {

	int index = hash(fd); 

	superblock_t superblock = drive.partitions[partition_index];
	file_table_entry_t entry = file_table[index];
	
	if(entry.inode == NULL) {

		file_table_entry_t new_entry;

		new_entry.inode = inode;
		new_entry.fpos = 0;
		new_entry.next = NULL;
		new_entry.prev = NULL;
		new_entry.fd = fd;
		new_entry.superblock = superblock;
		new_entry.drive = drive;

		file_table[index] = new_entry;

		return 0;
	}
	else {

		file_table_entry_t* entry_ptr = (file_table_entry_t*)(file_table + index);

		while(true) { 

			if(entry_ptr->next == NULL) {

				file_table_entry_t* new_entry = (file_table_entry_t*)kalloc(sizeof(file_table_entry_t));

				new_entry->inode = inode;
				new_entry->fpos = 0;
				new_entry->next = NULL;
				new_entry->prev = entry_ptr;
				new_entry->fd = fd;
				new_entry->superblock = superblock;
				new_entry->drive = drive;
				entry_ptr->next = new_entry;

				return 0; 
			}

			entry_ptr = entry_ptr->next;
		}
	}


	// will need to resize? 
	return -1;
}


void removeFromFileTable() { }