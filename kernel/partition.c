#include "./partition.h"

superblock_t* readSuperBlock(partition_entry_t* partition_table, drive_t* drive, int partition_index) {

	if(partition_table == NULL) {

		print("no partition table");

		return NULL;
	}

	partition_entry_t* part_entry = partition_table + partition_index;

	// if the partition entry doesn't exist return null
	if(part_entry->status != 0x80) {

		print("partition entry doesn't exist");

		return NULL;

	}

	// read the superblock 
	superblock_t* block = (superblock_t*)drive->read(1, part_entry->start_sector, *drive);

	if(block->magic_number != MAGIC_NUMBER) {

		print("invalid magic number: ");
		printi(block->magic_number);

		return NULL;
	}

	return block;
}

void updateSuperBlock(unsigned int meta_region, unsigned int data_region, unsigned int sector_count, unsigned int start_lba, drive_t* drive, int part_index) {

	drive->partitions[part_index].magic_number = MAGIC_NUMBER;
	drive->partitions[part_index].meta_region = meta_region;
	drive->partitions[part_index].data_region = data_region;
	drive->partitions[part_index].sector_count = sector_count;
	drive->partitions[part_index].start_lba = start_lba;
	drive->partitions[part_index].root_inode_lba = meta_region;

	int bitmap_size = (sector_count + 31) / 32 * sizeof(int);	
	int bitmap_sector_count = ceil(bitmap_size/512.0);
	int bitmap_start_sector = meta_region + 1;

	setSectors(bitmap_start_sector, bitmap_start_sector + bitmap_sector_count, 1, *drive);
	
	int* sector_bitmap = (int*)kalloc(bitmap_size);
	
	memset(sector_bitmap, bitmap_size, 0);

	drive->partitions[part_index].sector_bitmap = sector_bitmap;
	drive->partitions[part_index].bitmap_sector_count = bitmap_sector_count;
	drive->partitions[part_index].bitmap_start_sector = bitmap_start_sector;

	print("partition at index:  ");
	printi(part_index);
	println();

	print("meta_region:  ");
	printi(meta_region);
	println();

	print("meta_region:  ");
	printi(data_region);
	println();


	print("sector_count:  ");
	printi(sector_count);
	println();

	print("start_lba:  ");
	printi(start_lba);
	println();

	print("root lba:  ");
	printi(drive->partitions[part_index].root_inode_lba);
	println();

	drive->write(1, start_lba, &drive->partitions[part_index], *drive);
}

partition_entry_t* readPartitionTable(drive_t drive){

	mbr_t* mbr = (mbr_t*)drive.read(1, 0, drive);

	return mbr->partition_table;
}


void updatePartionTable(partition_entry_t* table, drive_t drive) {

	mbr_t* mbr = (mbr_t*)drive.read(1, 0, drive);

	for(char i = 0; i < 4; i++){

		mbr->partition_table[i] = table[i]; 
	}

	drive.write(1, 0, mbr, drive);
}