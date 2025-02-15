#include "./paging.h"
#include "./heap.c"

#define FRAME_SIZE 4096

//shifting, unsigned, aligment, and padding

void init_paging(){

    identity_page_table = (PageTableEntry*)alloc(1024 * 8); // 1024 entries, each entry is 8 bytes

    const int frame_limit = 0x100000 + (1024 * 8) + (1024 * 8);
    const int num_pages = frame_limit/FRAME_SIZE;

    int curr_frame = 0x00000;

   for (int i = 0; i < num_pages; i++) {

        PageTableEntry identity_pte = { 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, curr_frame >> 12 };

        identity_page_table[i] = identity_pte;

        curr_frame += frame_size;
    }
    
    unsigned int page_table_address = identity_page_table;


    PageDirectoryEntry page_directory_entry = { 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, page_table_address >> 12  };

    identity_page_directory = (PageDirectoryEntry*)alloc(1024 * 8);  // 1024 entries, each entry is 8 bytes

    //allocate page direcoty

    identity_page_directory[0] = page_directory_entry;

    enable_paging();
}











