#define BLOCK_SIZE 512

typedef struct {

	char* name;
	int a[120];
	unsigned int size;

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
	entry_t entries[(BLOCK_SIZE-sizeof(meta_t))/sizeof(entry_t)];

} inode_t;
