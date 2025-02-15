int heap_address = 0x100000;

//unit is in bytes
void* alloc(int size){

	void* starting_address = heap_address;

	heap_address += size;

	return starting_address;
}