#include "./paging.h"
#include "./heap.c"


PageTableEntry createPageTableEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int frame_addr){

    PageTableEntry pte = { p, w, u, pcd, ptw, 0, 0, 0, 0, 0, 0, 0, frame_addr >> 12 };

    return pte;
}


PageDirectoryEntry createPageDirectoryEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int table_addr){

    PageDirectoryEntry pde = { p, w, u, pcd, ptw, 0, 0, 0, 0, 0, 0, 0, table_addr >> 12 };

    return pde;
}

int getFrame(int frame_index){
    
    int remainder = frame_index % BITMAP_SIZE;
    
    int frame = frame_bitmap[frame_index/BITMAP_SIZE] >> remainder;
    
    return frame;
}

void setFrame(int frame_index, int present) {

    int remainder = frame_index % BITMAP_SIZE;
    int entry = frame_bitmap[frame_index/BITMAP_SIZE];
    
    // set the remainder'th bit to present 
    
    if (present == 1){
        
        frame_bitmap[frame_index/BITMAP_SIZE] = entry | (1 << remainder);
    }
    else{
        
        frame_bitmap[frame_index/BITMAP_SIZE] = entry &  ~(1 << remainder);
    }
    
}

void initPaging(){

   
   int frame_limit = 0x100000;
   int num_pages = frame_limit/FRAME_SIZE;
   int curr_frame = 0x00000;

   frame_index = 0;

   identity_page_table = (PageTableEntry*)alloc(1024*8);

   for (int i = 0; i < num_pages; i++) {

        //PageTableEntry identity_pte = { 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, curr_frame >> 12 };

        identity_page_table[i] = createPageTableEntry(1,1,0,0,1,curr_frame);
 
        setFrame(i, 1); // mark frame as present in the bit map

        curr_frame += FRAME_SIZE;
        frame_index+= 1;
    }

    unsigned int page_table_address = identity_page_table;

    //PageDirectoryEntry page_directory_entry = { 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, page_table_address >> 12  };
    
    PageDirectoryEntry page_directory_entry = createPageDirectoryEntry(1,1,0,0,1,page_table_address);
    
    identity_page_directory = (PageDirectoryEntry*)alloc(1024*8);

    //allocate page direcoty

    identity_page_directory[0] = page_directory_entry;

    enable_paging();

    
}

int lruSwap(){

    return 0;
}

int findFreeFrame(){

    int curr_index = frame_index;
    int entries_scanned = 0;

    while(true){

        if(getFrame(curr_index) == 0){ // free frame was found

            return curr_index;
        }

        if(entries_scanned*FRAME_SIZE > RAM_SIZE ){

            return lruSwap();
        }

        if(curr_index*FRAME_SIZE >= RAM_SIZE){

            curr_index = 0;
        }
        else{

            curr_index ++;
        }

        entries_scanned++;
    }
}

page_t* allocateFrame(){

    int free_frame_index = findFreeFrame();

    //set as used 
    setFrame(free_frame_index, 1);

    frame_index = free_frame_index;

    return (page_t*)(free_frame_index*FRAME_SIZE);

}



void handlePageFault(int virtual_address, int code){

    print("page fault");
    println();


    //first 12 bits
    unsigned int offset = (virtual_address & 0xFFFFF000) ^ virtual_address;

    //middle 10 bits 
    unsigned int pte_index = ((virtual_address & 0xFFC00FFF) ^ virtual_address) >> 12;

    //upper 10 bits
    unsigned int pde_index = ((virtual_address & 0x3FFFFF) ^ virtual_address) >> 22;


    if(0x2 & code) {

        // does the page direcotry entry exist?

        unsigned int page_table_address = identity_page_directory[pde_index].page_table_address; 

        if(page_table_address == 0x0){

            // create a new page table 
            PageTableEntry* page_table = (PageTableEntry*)alloc(1024 * 8);

            // add entry to the page table
            page_t* frame_addr = allocateFrame();
            page_table[pte_index] = createPageTableEntry(1,1,0,0,1,frame_addr);

            page_table_address = page_table;

            // create page directory entry
            PageDirectoryEntry page_directory_entry = createPageDirectoryEntry(1,1,0,0,1,page_table_address);
            identity_page_directory[pde_index] = page_directory_entry;

        }
        else {

            PageTableEntry* page_table = (PageTableEntry*)page_table_address;

            // add entry to the page table
            page_t* frame_addr = allocateFrame();
            page_table[pte_index] = createPageTableEntry(1,1,0,0,1,frame_addr);

        }
    }


    return;

    
}











