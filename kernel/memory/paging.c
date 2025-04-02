#include "./paging.h"
#include "./memory.h"
#include "./frame_manager.h"
#include "./utils.h"

#include <stdbool.h>

PageDirectoryEntry* page_directory;


PageTableEntry createPageTableEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int frame_addr){

    PageTableEntry pte = { p, w, u, pcd, ptw, 0, 0, 0, 0, 0, 0, 0, frame_addr >> 12 };

    return pte;
}


PageDirectoryEntry createPageDirectoryEntry(unsigned int p, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, unsigned int table_addr){

    PageDirectoryEntry pde = { p, w, u, pcd, ptw, 0, 0, 0, 0, 0, 0, 0, table_addr >> 12 };

    return pde;
}


void initPaging(int extra_memory_needed){

   int num_pages = (0x100000 + extra_memory_needed)/FRAME_SIZE; 

   // I want to change this, instead allocate on heap
   
   PageTableEntry* page_table = (PageTableEntry*)allocPage();

   memset(page_table, FRAME_SIZE, 0);

   page_directory = (PageDirectoryEntry*)allocPage();
  
   memset(page_directory, FRAME_SIZE, 0);

   for (int i = 0x0; i < num_pages; i++) {
            
        page_table[i] = createPageTableEntry(1, 1, 0, 0, 1, i * FRAME_SIZE);
 
        setFrame(i, 1); // mark frame as present in the bit map
    }

    page_directory[0] = createPageDirectoryEntry(1, 1, 0, 0, 1, (unsigned int)page_table);
    page_directory[1023] = createPageDirectoryEntry(1, 1, 0, 0, 1, (unsigned int)page_directory);

    enable_paging();

}




unsigned int virtAddressToPhysAddress(int virtual_address){

    unsigned int offset = virtual_address & 0x3FF;
    unsigned int pte_index = (virtual_address >> 12) & 0x3FF;
    unsigned int pde_index = (virtual_address >> 22) & 0x3FF;

    PageDirectoryEntry pd = page_directory[pde_index];

    if(pd.p != 0x0){

         PageTableEntry* page_table = (PageTableEntry*)(REC_PAGE_TABLE + (pde_index * FRAME_SIZE));

         if(page_table[pte_index].p == 0x0){

            return -1;
         }

         return (page_table[pte_index].page_frame_address << 12) + offset;
    }  

    return -1; 
}



void mapPage(unsigned int virtual_address, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw, bool identity_mapped){

    unsigned int pde_index = (virtual_address >> 22) & 0x3FF;
    unsigned int pte_index = (virtual_address >> 12) & 0x3FF;

    PageDirectoryEntry pde = page_directory[pde_index];

    if(pde.p == 0x0){

        page_t* page_table_frame = allocPage();
        page_directory[pde_index] = createPageDirectoryEntry(1, w, u, pcd, ptw, (unsigned int)page_table_frame);
    }

    PageTableEntry* page_table = (PageTableEntry*)(REC_PAGE_TABLE + (pde_index * FRAME_SIZE));

    if(identity_mapped) {

        page_table[pte_index] = createPageTableEntry(1, w, u, pcd, ptw, virtual_address);
    }
    else {

        page_t* frame_addr = allocPage();
        page_table[pte_index] = createPageTableEntry(1, w, u, pcd, ptw, (unsigned int)frame_addr);
    }

}

void identityMapPages(void* mem, unsigned int size, unsigned int w, unsigned int u, unsigned int pcd, unsigned int ptw){

    unsigned int start_index = (unsigned int)mem/FRAME_SIZE;
    unsigned int frames_needed = (size + FRAME_SIZE - 1) / FRAME_SIZE;

    for (int i = start_index; i < start_index + frames_needed; i++) { 
               
        unsigned int phys_addr = i * FRAME_SIZE;

        mapPage(phys_addr, w, u, pcd, ptw, true);
    }
}


void unmapPage(unsigned int virtual_address){

    unsigned int pde_index = (virtual_address >> 22) & 0x3FF;
    unsigned int pte_index = (virtual_address >> 12) & 0x3FF;

    PageDirectoryEntry pde = page_directory[pde_index];

    if(pde.p != 0x0) {

        PageTableEntry* page_table = (PageTableEntry*)(REC_PAGE_TABLE + (pde_index * FRAME_SIZE));

        page_table[pte_index].p = 0; 

        asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");
    }
}


void handlePageFault(int virtual_address, int code) {

    mapPage(virtual_address, 1, 1, 1, 1, false);
}





