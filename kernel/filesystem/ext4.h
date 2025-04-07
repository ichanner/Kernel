#define BLOCK_SIZE 512
#define DEV 5
#define INODE_NUM_ENTRIES (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t)
#define EXT_TABLE_NUM_ENTRIES BLOCK_SIZE/sizeof(entry_t)
#define DATA_META_RATIO .25


typedef enum {
	FILE,
	DIR
} file_type;

typedef struct {

	char* name;
	file_type type;
	unsigned int size;
	unsigned int lba;

} meta_t;

typedef struct {

	unsigned int start;
	unsigned int blocks;

} entry_t;

typedef struct {

	#ifdef DEV 
		entry_t entries[DEV];
	#else
		entry_t entries[BLOCK_SIZE/sizeof(entry_t)];
	#endif

} entry_table_t;

typedef struct {

	meta_t meta;
	
	#ifdef DEV 
		entry_t entries[DEV];
	#else
		entry_t entries[(BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t)];
	#endif


} inode_t;

typedef struct {
    entry_t* entries;
    int entry_table_sector;
    bool is_primary_table;
    int num_entries;
    int first_index;
} add_entry_state_t;


