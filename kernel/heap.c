int heap_address = 0x100000;

//int heap_address = 0x400000;


//unit is in bytes
void* alloc(int size){

	void* starting_address = heap_address;

	heap_address += size;

	return starting_address;
}


void memset(void* start, int size, int value){

	char* ptr = (char*)start;

	for(int i = 0; i < size; i++){

		ptr[i] = value; 
	}
}