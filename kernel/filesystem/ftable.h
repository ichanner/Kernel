typedef struct file_table_entry {

	void* inode;
	unsigned int fd; 
	unsigned int fpos;
	drive_t drive;
	superblock_t superblock;
	struct file_table_entry* next;
	struct file_table_entry* prev;

} file_table_entry_t;
