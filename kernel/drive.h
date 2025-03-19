typedef struct {

	unsigned int sector_count;
	unsigned int data_region;
	int* sector_bitmap;

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