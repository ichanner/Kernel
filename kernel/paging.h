
extern void enable_paging();

#define FRAME_SIZE 4096
#define RAM_SIZE 0x400000 //4MB ~ for now
#define BITMAP_SIZE 32 

typedef struct  {

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

}   PageDirectoryEntry;

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

}  PageTableEntry;

typedef unsigned int page_t;

int frame_bitmap[RAM_SIZE/(FRAME_SIZE*BITMAP_SIZE)];
int frame_index;

PageDirectoryEntry*  page_directory;

//PageTableEntry* identity_page_table;
