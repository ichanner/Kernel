#include "./ext4.c"

typedef struct buffer_head {

	unsigned int start;
	unsigned int blocks;
	bool dirty;
	char* data;
	struct buffer_head* next;
	struct buffer_head* prev;

} buffer_head_t;

void cache(inode_t*, char*, unsigned int, unsigned int);
char* checkCache(inode_t*, unsigned int);