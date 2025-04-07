
typedef struct  
{
	unsigned short magic_number;
	unsigned int data_region;
	unsigned int meta_region;
	unsigned int root_inode_lba;
	unsigned int sector_count;
	unsigned int start_lba;
	unsigned int bitmap_start_sector;
	unsigned int bitmap_sector_count;
	int* sector_bitmap;
	
} superblock_t;

typedef struct drive {

	unsigned int sector_count;
	unsigned short* (*read)(int, unsigned int, struct drive);
	void (*write)(int, unsigned int, void*, struct drive);

	superblock_t partitions[4];

	union {

		struct {

			unsigned short port_base;
			unsigned int dma_mode;
			unsigned char is_master;
			unsigned char is_28_bit;
		};
	};


} drive_t;


drive_t drives[4]; //temporary 