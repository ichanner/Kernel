#ifndef FRAME_MANAGER_H
#define FRAME_MANAGER_H

#include "./memory.h"


#define HEADER_OFFSET 4


int getFrame(int);
void setFrame(int, int);
page_t* allocPage();
page_t* allocPages(int);

int allocContigousFrames(int);


#endif