#define BLOCK_SIZE 512.0
#define DEV 5
#define INODE_NUM_ENTRIES (BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t)
#define EXT_TABLE_NUM_ENTRIES BLOCK_SIZE/sizeof(entry_t)
#define DATA_META_RATIO .25


typedef struct buffer_head {

	unsigned int start;
	unsigned int blocks;
	bool dirty;
	char* data;
	struct buffer_head* next;
	struct buffer_head* prev;

} buffer_head_t;

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

	buffer_head_t* buffer_head;


} inode_t;

typedef struct {
	
	int i; 
	int entry_table_sector;
    int num_entries;
    bool is_primary_table;
	entry_t* entries; 

} entry_iterator_t;

typedef struct {
    entry_t* entries;
    int entry_table_sector;
    bool is_primary_table;
    int num_entries;
    int first_index;
} add_entry_state_t;

typedef struct {
    entry_t* entries;
    int num_entries;
    int first_index;
} get_entry_state_t;
