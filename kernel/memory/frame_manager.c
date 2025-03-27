#include "./memory.h"
#include "./frame_manager.h"
#include "./utils.h"
#include <stddef.h>
#include <stdbool.h>

int* frame_bitmap;
int frame_ptr;

int initFrameManager(){

   int bitmap_size = (total_ram/(FRAME_SIZE*32)); 
   
   frame_bitmap = (int*)kalloc(bitmap_size * sizeof(int));

   memset(frame_bitmap, bitmap_size * sizeof(int), 0);

   return (bitmap_size <= FRAME_SIZE) ? FRAME_SIZE : ((bitmap_size + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1));
}

int getFrame(int frame_index){
    
    int remainder = frame_index % 32;
    int frame = (frame_bitmap[frame_index/32] >> remainder) & 0x1;
    
    return frame;
}

void setFrame(int frame_index, int present) {

    int remainder = frame_index % 32;
    int entry = frame_bitmap[frame_index/32];
    
    // set the remainder'th bit to present 
    
    if (present == 1){
        
        frame_bitmap[frame_index/32] = entry | (1 << remainder);
    }
    else{
        
        frame_bitmap[frame_index/32] = entry &  ~(1 << remainder);
    }

    frame_ptr = frame_index;
}

void setFrames(int start_index, int end_index, int present){

    for(int i = start_index; i < end_index; i++){

        setFrame(i, present);
    }
}

page_t* allocPage(){

    int entries_scanned = 0;
    int curr_frame = frame_ptr;

    while(true){

        if(getFrame(curr_frame) == 0){ // free frame was found

            setFrame(curr_frame, 1);

            return curr_frame * FRAME_SIZE;
        }

        if(entries_scanned * FRAME_SIZE > total_ram) {

            break;
        }
        else if(curr_frame * FRAME_SIZE >= total_ram){

            curr_frame = 0;
        }
        else{

            curr_frame ++;
        }

        entries_scanned++;
    }

    return NULL;
}

page_t* allocPages(int size) {
   
    int frames_needed = (size + FRAME_SIZE - 1) / FRAME_SIZE;  // Correct rounding
    int index = frame_ptr;  
    int max_index = total_ram / FRAME_SIZE;

    while (index < max_index) {
        
        if (getFrame(index) == 0) {  // Found a free frame
           
            int free_frames = 1;
            int free_frame_index = index + 1;

            while (free_frame_index < max_index && getFrame(free_frame_index) == 0) {
               
                free_frames++;
                free_frame_index++;
            }

            if (free_frames >= frames_needed) {  // Found enough contiguous frames

                setFrames(index, index + frames_needed, 1);

                return (page_t*)(index * FRAME_SIZE);
            }
        
            index += free_frames;  // Skip past checked free frames
       
        } else {
           
            index++;
        }
    }

    return NULL;  // No available contiguous frames
}


/*



int allocContigousFrames(int size) {
   
    int frames_needed = (size + FRAME_SIZE - 1) / FRAME_SIZE;  // Correct rounding
    int index = frame_ptr;  // DMA Zone
    int max_index = total_ram / FRAME_SIZE;

    while (index < max_index) {
        
        if (getFrame(index) == 0) {  // Found a free frame
           
            int free_frames = 1;
            int free_frame_index = index + 1;

            while (free_frame_index < max_index && getFrame(free_frame_index) == 0) {
                free_frames++;
                free_frame_index++;
            }

            if (free_frames >= frames_needed) {  // Found enough contiguous frames
                for (int i = index; i < index + frames_needed; i++) {  // Only allocate needed frames
                    unsigned int phys_addr = i * FRAME_SIZE;


                    mapPage(phys_addr, 1, 0, 0, 1);
                    setFrame(i, 1);
                }
                return index * FRAME_SIZE;
            }
        
            index += free_frames;  // Skip past checked free frames
       
        } else {
           
            index++;
        }
    }

    return -1;  // No available contiguous frames
}
*/
