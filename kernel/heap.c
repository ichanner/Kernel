int heap_address;

void initHeap(unsigned int addr){

	heap_address = addr;
}

//unit is in bytes
void* alloc(int size){

	void* starting_address = heap_address;

	heap_address += size;

	return starting_address;
}


//REMOVE
void memset(void* start, int size, int value){

	char* ptr = (char*)start;

	for(int i = 0; i < size; i++){

		ptr[i] = value; 
	}
}