typedef struct {

	unsigned short port_base;
	unsigned short disk_id;
	unsigned int sector_count;
	unsigned int data_region;
	int* sector_bitmap;

} disk_t;

disk_t disks[4]; //temporary 