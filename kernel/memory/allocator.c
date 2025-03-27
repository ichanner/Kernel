#include "./allocator.h"
#include "./memory.h"
#include <stddef.h>
/*

    SetFrame back to 0 will create new item in free list, merge any adjacent free's together
*/

block_t* free_list_root;
//block_t* frame_list_root;


void print_free_list() {
    // Check if the free list is empty
    

    // Header
    print("Free List: Index | Size | Address | Prev | Next");
    println();

    block_t* current = free_list_root;
    int index = 0;

    // Traverse the free list
    while (1) {
        // Print index
        printi(index);
        print(" | ");

        // Print size
        printi(current->size);
        print(" | ");

        // Print current address (as a hex-like integer for simplicity)
        printi((int)current);
        print(" | ");

        // Print prev address
        printi((int)current->prev);
        print(" | ");

        // Print next address
        printi((int)current->next);
        println();

        current = current->next;

        if(current == NULL) break;
        index++;

       
    }
}

void initAlloc(){

    free_list_root = (block_t*)0x100000;
    free_list_root->size = total_ram - 0x100000;
    free_list_root->prev = NULL;
    free_list_root->next = NULL;
    
    //frame_list_root->size = total_ram;
   // frame_list_root->prev = NULL;
   // frame_list_root->next = NULL;
    //frame_list_root = (block_t*)0x0;
}


void free(void* ptr){

    unsigned int block_addr = ((unsigned int)ptr - HEADER_SIZE); 
    
    block_t* free_list_ptr = free_list_root;
    block_t* freed_block = (block_t*)block_addr;

    if(free_list_ptr == NULL) {

        freed_block->prev = NULL;
        freed_block->next = NULL;

        free_list_root = freed_block;

       //  print("case 1");

        return;
    }

    if(free_list_ptr->size >= freed_block->size){

        freed_block->prev = NULL;
        freed_block->next = free_list_ptr;
        free_list_ptr->prev = freed_block;
        free_list_root = freed_block;

      //  print("case 2");

        return;
    }


   /* println();
    print("SIZE: ");
    printi(freed_block->size);
*/
    while(1){

        if(free_list_ptr->size < freed_block->size && free_list_ptr->next != NULL) {   

            free_list_ptr = free_list_ptr->next;

            continue;

        }
        
        freed_block->prev = free_list_ptr;
        freed_block->next = free_list_ptr->next;

        if(free_list_ptr->next != NULL){

            free_list_ptr->next->prev = freed_block;
        }

        free_list_ptr->next = freed_block;

        break;

            /*

            if(free_list_ptr->next != NULL && ((ptr + freed_size) == free_list_ptr->next)){


            }   
         
            if(free_list_ptr != NULL && (block_addr == ((free_list_ptr + HEADER_SIZE) + free_list_ptr->size))){


            }

            */
    
    }    

}


void* kalloc(unsigned int req_size) {

    block_t* free_list_ptr = free_list_root;

    while(1){

        unsigned int total_size = free_list_ptr->size;
        unsigned int padding = 0;

        if(total_size >= req_size){ // it matches requested size 

            unsigned int end_address = (unsigned int)free_list_ptr + req_size + HEADER_SIZE;

            padding += (HEAP_ALIGNMENT - (end_address % HEAP_ALIGNMENT)) % HEAP_ALIGNMENT;
            end_address += padding;

            unsigned int buddy_size = (total_size - (req_size + padding));


            if(total_size != req_size && buddy_size >= HEADER_SIZE) { // still left over room to make new free entry


                block_t* buddy = (block_t*)end_address;

                buddy->size = buddy_size;
                buddy->prev = free_list_ptr->prev; // buddy prev points to original prev 
                buddy->next = free_list_ptr->next; // buddy next points to original next 

                if(free_list_ptr->prev != NULL) {

                    free_list_ptr->prev->next = buddy; // update allocaed prev next pointer to buddy
                }

                if(free_list_ptr->next != NULL) {

                    free_list_ptr->next->prev = buddy; // update allocaed next prev pointer to buddy
                }

                if(free_list_ptr == free_list_root) {

                    free_list_root = buddy;
                 }

            }
            else {

                if(free_list_ptr->next != NULL){

                    free_list_ptr->next->prev = free_list_ptr->prev;
                }

                if(free_list_ptr->prev != NULL) {

                    free_list_ptr->prev->next = free_list_ptr->next;

                }

                if(free_list_ptr == free_list_root) free_list_root = free_list_ptr->next;

            }

            free_list_ptr->size = req_size + padding; // set the new size 

            return (void*)((unsigned int)free_list_ptr + HEADER_SIZE);
        }
        else {

            if(free_list_ptr->next != NULL){ // go to next entry in free list

                free_list_ptr = free_list_ptr->next;

            }
            else {

                break;
            }
        }
    }

    return NULL;
}







/*



void* palloc() {

    block_t* free_list_ptr = frame_list_root;

    while(1){

        unsigned int total_size = free_list_ptr->size;
        
        if(total_size >= 4096){ // it matches requested size 

            unsigned int start_address = (unsigned int)free_list_ptr;
            

            //unsigned int padding = (PAGE_ALIGNMENT - (start_address % PAGE_ALIGNMENT)) % PAGE_ALIGNMENT;
          
            //start_address += padding;

            unsigned int end_address = (start_address + 4096) ;
            unsigned int buddy_size = total_size - (start_address + 4096);

            if(total_size != 4096 && buddy_size >= HEADER_SIZE) { // still left over room to make new free entry

                block_t* buddy = (block_t*)end_address;

                buddy->size = buddy_size;
                buddy->prev = free_list_ptr->prev; // buddy prev points to original prev 
                buddy->next = free_list_ptr->next; // buddy next points to original next 

                if(free_list_ptr->prev != NULL) {

                    free_list_ptr->prev->next = buddy; // update allocaed prev next pointer to buddy
                }

                if(free_list_ptr->next != NULL) {

                    free_list_ptr->next->prev = buddy; // update allocaed next prev pointer to buddy
                }

                if(free_list_ptr == frame_list_root) {

                    frame_list_root = buddy;
                 }

            }
            else {

                if(free_list_ptr->next != NULL){

                    free_list_ptr->next->prev = free_list_ptr->prev;
                }

                if(free_list_ptr->prev != NULL) {

                    free_list_ptr->prev->next = free_list_ptr->next;

                }

                if(free_list_ptr == frame_list_root) frame_list_root = free_list_ptr->next;

            }

            free_list_ptr->size = 4096; // set the new size

             printi((start_address)%4096);

            print(",");
            
            printi((start_address)/4096);

            print(" ");

            return (void*)(start_address + HEADER_SIZE);
        }
        else {

            if(free_list_ptr->next != NULL){ // go to next entry in free list

                free_list_ptr = free_list_ptr->next;

            }
            else {

                break;
            }
        }
    }

    return NULL;
}*/
