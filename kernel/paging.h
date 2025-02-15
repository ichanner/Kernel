

extern void enable_paging();

typedef struct {

	unsigned int p : 1;
	unsigned int w : 1;
	unsigned int u : 1;
	unsigned int pcd : 1;
	unsigned int ptw : 1;
	unsigned int a : 1;
	unsigned int ignored_6 : 1;
	unsigned int ps : 1;
	unsigned int ignored_8 : 1;
	unsigned int ignored_9 : 1;
	unsigned int ignored_10 : 1;
	unsigned int ignored_11 : 1;
	unsigned int page_table_address : 20;

}  __attribute__((packed)) PageDirectoryEntry;

typedef struct {

	unsigned int p : 1;
	unsigned int w : 1;
	unsigned int u : 1;
	unsigned int pcd : 1;
	unsigned int ptw : 1;
	unsigned int a : 1;
	unsigned int dirty : 1;
	unsigned int ignored_7 : 1;
	unsigned int ignored_8 : 1;
	unsigned int ignored_9 : 1;
	unsigned int ignored_10 : 1;
	unsigned int ignored_11 : 1;
	unsigned int page_frame_address : 20;

} __attribute__((packed)) PageTableEntry;


PageDirectoryEntry* identity_page_directory;
PageTableEntry* identity_page_table;


///PageDirectoryEntry identity_page_directory[1024] __attribute__((aligned(4096)));
//PageTableEntry identiy_page_table[1024] __attribute__((aligned(4096)));

