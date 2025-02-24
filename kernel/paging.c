#include "./paging.h"
#include "./heap.c"


/*
    
    TODO:

        implement LRU algorithm
            - finding least recently used frame
            - check dirty bit and swap it to frame


        do page directory context swtich 

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

void clearAccessBits(){

    for(int i = 0; i < 1024; i++) {

        PageDirectoryEntry pde = page_directory[i];

        PageTableEntry* pt = (PageTableEntry*)pde.page_table_address;

        if(pde.p != 0x0){

            for(int j = 0; j < 1024; j++) {

                PageTableEntry pte = pt[j]; 

                if(pte.p != 0x0){

                    unsigned int a = pte.a;

                    int lru_score = lru_scores[pte.page_frame_address/FRAME_SIZE];

                    if(a == 0x1){

                        lru_scores[pte.page_frame_address/FRAME_SIZE] = lru_score / 2;
                    }
                    else{

                        lru_scores[pte.page_frame_address/FRAME_SIZE] = lru_score + 1;
                    }

                    pt[j].a = 0;
                }
            }
        }   
    }
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


/*

    Currently: 

       We map the first page table to itself because the first page table maps the page directory 

        - To get page directory: First Page Table -> Page Directory (therefore both must be mapped as they rely on eachother)
    

    Next:

        We now want to just map the page directory to itself withotu intermediate page table loookup 
*/

void initPaging(){
   
   int frame_limit = 0x100000; //Map up to 4MB 
   int num_pages = frame_limit/FRAME_SIZE; 
   
   PageTableEntry*  page_table = (PageTableEntry*)allocateFrame();

   memset(page_table, 4096, 0);

   page_directory = (PageDirectoryEntry*)allocateFrame();

   memset(page_directory, 4096, 0);

   for (int i = 0; i < num_pages; i++) {
        
        page_table[i] = createPageTableEntry(1, 1, 0, 0, 1, 0, i * FRAME_SIZE);
 
        setFrame(i, 1); // mark frame as present in the bit map
        
        frame_index += 1;
    }

    page_directory[0] = createPageDirectoryEntry(1, 1, 0, 0, 1, (unsigned int)page_table);
  
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


void handlePageFault(int virtual_address, int code){

   //  print("page fault");
   // println();

    //first 12 bits
    unsigned int offset = (virtual_address & 0xFFFFF000) ^ virtual_address;

    //middle 10 bits 
    unsigned int pte_index = ((virtual_address & 0xFFC00FFF) ^ virtual_address) >> 12;

    //upper 10 bits
    unsigned int pde_index = ((virtual_address & 0x3FFFFF) ^ virtual_address) >> 22;

/*
    println();
    print("pte_index: ");
    printi(pte_index);
    println();
    print("pde_index: ");
    printi(pde_index);
    println();
*/

   // if(0x2 & code) {

        PageDirectoryEntry pd = page_directory[pde_index];
        
        unsigned int page_table_address = pd.page_table_address << 12;
        unsigned int present = pd.p;

        if(present == 0x0) { // if pde isn't present 

          //  println();
       //     print("Creating new page table");

            // create a new page table from allocated frame
            PageTableEntry* page_table = (PageTableEntry*)allocateFrame();
            
            // assign frame address for new page table to new page directory entry
            PageDirectoryEntry page_directory_entry = createPageDirectoryEntry(1,1,0,0,1, (unsigned int) page_table);
            
            // assign new pde to page directory
            page_directory[pde_index] = page_directory_entry;

            // allocate a new frame
            page_t* frame_addr = allocateFrame();

            // assign frame to new page table entry 
            page_table[pte_index] = createPageTableEntry(1, 1, 0, 0, 1, 0, frame_addr);

        }
        else {


         //   println();
         //   print("Page table found");

            // get page table from page table address 
            PageTableEntry* page_table = (PageTableEntry*)page_table_address;
            
            // allocate a new frame
            page_t* frame_addr = allocateFrame();

            // assign frame to new page table entry 
            page_table[pte_index] = createPageTableEntry(1,1,0,0,1,0,frame_addr);
        
        }
        
        
   // }




    
}











