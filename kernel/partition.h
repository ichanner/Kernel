#define MAGIC_NUMBER 0xEF53

typedef struct {

	unsigned char status;
	unsigned char start_sector;
	unsigned char type;
	unsigned char size;

} partition_entry_t; 

typedef struct {

	unsigned char bootloader[494];
	partition_entry_t partition_table[4];
	unsigned short signature; 

} mbr_t;