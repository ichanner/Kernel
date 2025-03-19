#include "./paging.h"
#include "./heap.c"


/*
    
    TODO:

       test lru calculation

       write a disk manager 
        -manaing partitions
        -bitmap of free sectors

       lru swap 

*/



/*

Aging clock algorithm



    Every round 
        - if access bit is 0: score +1
        - if access bit is 1: score / 2 
        - initialze new scores to 1
        - closer to 0 indicates most used closer to inf indicates least used 

        we dont care about page tables entries that aren't present because they don't map to a valid physical address
        we dont care about page directory entries that aren't presnet because they dont map to a valid page table 
*/




void initialzeBackingStoreTable(){}

void calculateLRU(){

    //initialize max score to -1
    int max_lru_score = -1;

    //iterate over page directory entries 
    for(int i = 0; i < 1024; i++) {

        // get the page directory entry 
        PageDirectoryEntry pde = page_directory[i];

        // only continue if the pde is valid 
        if(pde.p != 0x0) {

            //get the associated page table
            PageTableEntry* pt = (PageTableEntry*)pde.page_table_address;

            //iterate over each page table entry
            for(int j = 0; j < 1024; j++) {

                //get the entry 
                PageTableEntry pte = pt[j]; 

                //only continue if  valid
                if(pte.p != 0x0){

                    // access bit 
                    unsigned int a = pte.a;

                    // get the lru score 
                    int lru_score = lru_scores[pte.page_frame_address/FRAME_SIZE];

                    if(a == 0x1){

                        lru_scores[pte.page_frame_address/FRAME_SIZE] = lru_score / 2;
                    }
                    else{

                        lru_scores[pte.page_frame_address/FRAME_SIZE] = lru_score + 1;
                    }

                    lru_score = lru_scores[pte.page_frame_address/FRAME_SIZE];

                    if(lru_score > max_lru_score){

                        max_lru_score = lru_score;

                        lru_pte_index =  j;
                        lru_pde_index =  i;

                     
                    }

                    pt[j].a = 0;
                }
            }
        }   
    }

    /*
    println();
    print("LRU PTE INDEX: ");
    printi(lru_pte_index);
    println();
    print("LRU PDE INDEX: ");
    printi(lru_pde_index);
    */

}

page_t* allocateFrame(){

    int free_frame_index = findFreeFrame();

    //set as used 
    setFrame(free_frame_index, 1);

    frame_index = free_frame_index;

    return (page_t*)(free_frame_index*FRAME_SIZE);

}

PageTableEntry createPageTableEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int dirty, unsigned int frame_addr){

    PageTableEntry pte = { p, w, u, pcd, ptw, 0, dirty, 0, 0, 0, 0, 0, frame_addr >> 12 };

    return pte;
}


PageDirectoryEntry createPageDirectoryEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int table_addr){

    PageDirectoryEntry pde = { p, w, u, pcd, ptw, 0, 0, 0, 0, 0, 0, 0, table_addr >> 12 };

    return pde;
}

int getFrame(int frame_index){
    
    int remainder = frame_index % BITMAP_SIZE;
    
    int frame = (frame_bitmap[frame_index/BITMAP_SIZE] >> remainder) & 0x1;
    
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



void mapPage(int phys_addr, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw){

    unsigned int pte_index = (phys_addr >> 12) & 0x3FF;
    unsigned int pde_index = (phys_addr >> 22) & 0x3FF; 
    unsigned int page_table_addr = page_directory[pde_index].page_table_address;
    unsigned int present = page_directory[pde_index].p;

    if(present == 0x0) {

        page_t* page_table_frame = allocateFrame();
        PageDirectoryEntry page_directory_entry = createPageDirectoryEntry(1, w, u, pcd, ptw, page_table_frame);
        page_directory[pde_index] = page_directory_entry;
    }  

    unsigned int page_table_virt_addr = 0xFFC00000 + (pde_index * FRAME_SIZE);
    PageTableEntry* page_table = (PageTableEntry*)page_table_virt_addr;
    page_table[pte_index] = createPageTableEntry(1, w, u, pcd, ptw, 0, phys_addr);
}

int allocContigousFrames(int size){

    int frames_needed = ceil(size/FRAME_SIZE);
    int index = 0; //DMA_ZONE
    int max_index = total_ram/FRAME_SIZE;

    while(1) {

        if(index >= max_index) {

            break;
        }
        else if(getFrame(index) == 0){
            
            int free_frames = 1;
            int free_frame_index = index + 1;

            while(getFrame(free_frame_index) != 1 && free_frame_index < max_index) {

                free_frames++;
                free_frame_index++;
            }

            if(free_frames >= frames_needed) {

                for(int i = index; i < index + free_frames; i++){

                    unsigned int phys_addr = i * FRAME_SIZE;
                    
                    mapPage(phys_addr, 1, 0, 0, 1);

                    setFrame(i, 1);

                }

                return index * FRAME_SIZE;
            }
        
            index += free_frames;
        }
        else{

            index += 1; 
        }
    }

    return -1;
}


int getRAM(){

    int ram_acc = 0;
    
    char* memory_map = (char*)0x8000;

    while(1) {

        MemoryMapEntry* entry = (MemoryMapEntry*)memory_map;

        ram_acc += entry->length_lower;
       
        memory_map += 24; // each entry is 24 bytes long

        if(*memory_map == 0x2c){ // 0x2c is termination number

            print("TOTAL RAM: ");
            printi(ram_acc/(1024*1024));
            print(" MB");
            println();

           // lru_scores = (int*)alloc(ram_acc/FRAME_SIZE);

            break;
        }
    } 

    return ram_acc;
}

void initPaging(){

   total_ram = getRAM();
   
   int bitmap_size = (total_ram/(FRAME_SIZE*32));
    
   bitmap_size = (bitmap_size <= FRAME_SIZE) ? FRAME_SIZE : ((bitmap_size + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1));

   lru_pde_index = 0;
   lru_pte_index = 0;

   memset(&lru_scores, RAM_SIZE/FRAME_SIZE, 1);

   int frame_limit = 0x100000 + bitmap_size; //Map up to 4MB 
   int num_pages = frame_limit/FRAME_SIZE; 
   
   PageTableEntry* page_table = (PageTableEntry*)allocateFrame();
   memset(page_table, 4096, 0);

   page_directory = (PageDirectoryEntry*)allocateFrame();
   memset(page_directory, 4096, 0);

   for (int i = 0; i < num_pages; i++) {
        
        page_table[i] = createPageTableEntry(1, 1, 0, 0, 1, 0, i * FRAME_SIZE);
 
        setFrame(i, 1); // mark frame as present in the bit map
        
        frame_index += 1;
    }

    page_directory[0] = createPageDirectoryEntry(1, 1, 0, 0, 1, (unsigned int)page_table);
    page_directory[1023] = createPageDirectoryEntry(1, 1, 0, 0, 1, (unsigned int)page_directory);

    enable_paging();
    
}

int lruSwap(){

    /*
        1. get page table entry associated lru index
    */


    PageDirectoryEntry lru_pde = page_directory[lru_pde_index];
    PageTableEntry* lru_pt = (PageTableEntry*)lru_pde.page_table_address;
    PageTableEntry lru_pte = lru_pt[lru_pte_index];

    /*
        2. if it was modified then write back to swap partition  

          

        3. 
    */

    if(lru_pte.dirty == 0x1) {

    }
    
    return 0;

    //return lru_pte.page_frame_address/FRAME_SIZE;
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


unsigned int physAddressToVirtualAddress(int phys_adddress){


}

unsigned int virtAddressToPhysAddress(int virtual_address){

    unsigned int offset = (virtual_address & 0xFFFFF000) ^ virtual_address;
    unsigned int pte_index = ((virtual_address & 0xFFC00FFF) ^ virtual_address) >> 12;
    unsigned int pde_index = ((virtual_address & 0x3FFFFF) ^ virtual_address) >> 22;

    PageDirectoryEntry pd = page_directory[pde_index];

    unsigned int page_table_address = pd.page_table_address << 12;
   
    if(pd.p != 0x0){

         //PageTableEntry* page_table = (PageTableEntry*)page_table_address;

        unsigned int page_table_virtual_addr = 0xFFC00000 + (pde_index * FRAME_SIZE);

        PageTableEntry* page_table = (PageTableEntry*)page_table_virtual_addr;

         if(page_table[pte_index].p == 0x0){

            handlePageFault(virtual_address, 0);
         }
  
         return (page_table[pte_index].page_frame_address << 12) + offset;
    }   
}


void handlePageFault(int virtual_address, int code){

    print("page fault");

    //first 12 bits
    unsigned int offset = (virtual_address & 0xFFFFF000) ^ virtual_address;

    //middle 10 bits 
    unsigned int pte_index = ((virtual_address & 0xFFC00FFF) ^ virtual_address) >> 12;

    //upper 10 bits
    unsigned int pde_index = ((virtual_address & 0x3FFFFF) ^ virtual_address) >> 22;


   // if(0x2 & code) {

        PageDirectoryEntry pd = page_directory[pde_index];
            
        unsigned int page_table_address = pd.page_table_address;
        unsigned int present = pd.p;

        if(present == 0x0) { // if pde isn't present 

            print(" Creating new page table ");

            // allocate frame for page table
            page_t* page_table_frame = allocateFrame();
            
            // assign frame address for new page table to new page directory entry
            PageDirectoryEntry page_directory_entry = createPageDirectoryEntry(1, 1, 0, 0, 1, page_table_frame);
            
            // assign new pde to page directory
            page_directory[pde_index] = page_directory_entry;

            // allocate a new frame
            page_t* frame_addr = allocateFrame();

            unsigned int page_table_virtual_addr = 0xFFC00000 + (pde_index * FRAME_SIZE);
            PageTableEntry* page_table = (PageTableEntry*)page_table_virtual_addr;

            // assign frame to new page table entry 
            page_table[pte_index] = createPageTableEntry(1, 1, 0, 0, 1, 0, frame_addr);

        }
        else {

            unsigned int page_table_virtual_addr = 0xFFC00000 + (pde_index * FRAME_SIZE);

            PageTableEntry* page_table = (PageTableEntry*)page_table_virtual_addr;
            
            // allocate a new frame
            page_t* frame_addr = allocateFrame();

            // assign frame to new page table entry 
            page_table[pte_index] = createPageTableEntry(1, 1, 0, 0, 1, 0, frame_addr);

        }
        
        
   // }

}











