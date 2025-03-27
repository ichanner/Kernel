#include "./memory.h"
#include "./swap.h"
#include "./paging.h"
#include "./frame_manager.h"

unsigned int total_ram;

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


void initMemory(){

    total_ram = getRAM();

    int additional_space = 0;

    initAlloc();
    
    additional_space += initFrameManager();
    additional_space = (additional_space <= FRAME_SIZE) ? FRAME_SIZE : ((additional_space + FRAME_SIZE - 1) & ~(FRAME_SIZE - 1));

    initPaging(additional_space);


}