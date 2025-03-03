#define BLOCK_SIZE 512

typedef struct {

	char* file_name;
	int a[120];

	unsigned int file_size;

} meta_t;

typedef struct {

		int a[20];

	unsigned int start;
	unsigned int blocks;

} entry_t;

typedef struct {

	entry_t entries[BLOCK_SIZE/sizeof(entry_t)];

} entry_table_t;

typedef struct {

	meta_t* file_meta;
	//char* a;
	entry_t entries[(BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t)];

} inode_t;
