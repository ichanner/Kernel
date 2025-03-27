#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#define HEADER_SIZE sizeof(int) * 3
#define PAGE_ALIGNMENT 4096
#define HEAP_ALIGNMENT 8
typedef struct block {
	
	unsigned int size;
	struct block* prev;
	struct block* next;

} block_t;

#endif 