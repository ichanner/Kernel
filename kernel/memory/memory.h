#ifndef MEMORY_H
#define MEMORY_H

#define FRAME_SIZE 4096

extern unsigned int total_ram;

typedef unsigned int page_t;

typedef struct {

	unsigned int base_addr_lower;
	unsigned int base_addr_upper;
	unsigned int length_lower;
	unsigned int length_upper;
	unsigned int region_type;
	unsigned int region_extended;

} MemoryMapEntry;

int getRAM();

#endif