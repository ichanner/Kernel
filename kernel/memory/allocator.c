#include "./allocator.h"
#include "./memory.h"
#include <stddef.h>
/*

    SetFrame back to 0 will create new item in free list, merge any adjacent free's together
*/

block_t* free_list_root;



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
    
}


void kfree(void* ptr){

    block_t* free_list_ptr = free_list_root;
    block_t* freed_block = (block_t*)((char*)ptr - HEADER_SIZE);;

    if(free_list_ptr == NULL) {

        freed_block->prev = NULL;
        freed_block->next = NULL;

        free_list_root = freed_block;

        return;
    }

    if(free_list_ptr->size >= freed_block->size){

        freed_block->prev = NULL;
        freed_block->next = free_list_ptr;
        free_list_ptr->prev = freed_block;
        free_list_root = freed_block;

        return;
    }


    while(1){

        if(free_list_ptr->size < freed_block->size && free_list_ptr->next != NULL) {   

            free_list_ptr = free_list_ptr->next;

            continue;

        }
        
        freed_block->prev = free_list_ptr;
        freed_block->next = free_list_ptr->next;
        
        if(free_list_ptr->next != NULL){

            free_list_ptr->next->prev = freed_block;

            if(((char*)ptr + freed_block->size) == (char*)free_list_ptr->next){

                // merge with next free block

                freed_block->next = free_list_ptr->next->next;
                freed_block->size += (HEADER_SIZE + free_list_ptr->next->size);

                if(freed_block->next != NULL){

                    freed_block->next->prev = freed_block;
                }
            }  
        }

        free_list_ptr->next = freed_block;
     
        if((char*)freed_block == ((char*)free_list_ptr + HEADER_SIZE + free_list_ptr->size)){

            // merge with previous 

            free_list_ptr->size += (HEADER_SIZE + freed_block->size);
            free_list_ptr->next = freed_block->next;

            if(freed_block->next != NULL){

                freed_block->next->prev = free_list_ptr;
            }
        }

        break;
            
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






