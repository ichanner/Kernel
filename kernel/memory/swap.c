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

#include "./swap.h"
#include "./frame_manager.h"
#include "./utils.h"
#include "./memory.h"

int* lru_scores;
int lru_pte_index;
int lru_pde_index;

int initSwap() {

    /*
    int lru_scores_size = total_ram/FRAME_SIZE * 32;

    lru_scores = (int*)alloc(lru_scores_size);

    memset(&lru_scores, lru_scores_size, 1);

    return lru_scores_size;
    */
}

int lruSwap(){

    /*
        1. get page table entry associated lru index
    */
/*

    PageDirectoryEntry lru_pde = page_directory[lru_pde_index];
    PageTableEntry* lru_pt = (PageTableEntry*)lru_pde.page_table_address;
    PageTableEntry lru_pte = lru_pt[lru_pte_index];
*/
    /*
        2. if it was modified then write back to swap partition  

          

        3. 
    */

   // if(lru_pte.dirty == 0x1) {

   // }
    
    return 0;

    //return lru_pte.page_frame_address/FRAME_SIZE;
}

void calculateLRU(){

    /*

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

    */
}