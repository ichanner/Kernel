#ifndef PAGING_H
#define PAGING_H
#define REC_PAGE_TABLE 0xFFC00000

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


extern void enable_paging();
extern PageDirectoryEntry* page_directory;

void initPaging(int);
unsigned int virtAddressToPhysAddress(int);
void handlePageFault(int, int);


#endif

