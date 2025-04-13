#include "./buffer.h"

void cache(inode_t* inode, char* data, unsigned int start, unsigned int blocks) {

	if(inode == NULL) return;

	buffer_head_t* buffer_head = inode->buffer_head;

	if(buffer_head == NULL) {

		buffer_head = (buffer_head_t*)kalloc(sizeof(buffer_head_t));

		buffer_head->start = start;
		buffer_head->blocks = blocks;
		buffer_head->data = data;
		buffer_head->dirty = true;
		buffer_head->prev = NULL;
		buffer_head->next = NULL;

		inode->buffer_head = buffer_head;
	}
	else {

		new_buffer_head = (buffer_head_t*)kalloc(sizeof(buffer_head_t));
		new_buffer_head->start = start;
		new_buffer_head->blocks = blocks;
		new_buffer_head->dirty = true;
		new_buffer_head->data = data;
		new_buffer_head->prev = NULL;
		new_buffer_head->next = buffer_head;

		buffer_head->prev = new_buffer_head;

		inode->buffer_head = new_buffer_head;
	}
}

char* checkCache(inode_t* inode, unsigned int start) {

	if(inode == NULL) return NULL;

	buffer_head_t* buffer_head = inode->buffer_head;

	while(buffer_head->next != NULL) {

		if(buffer_head->start == start) {

			return buffer_head->data;
		}	

		buffer_head = buffer_head->next;
	}

	return NULL;
}