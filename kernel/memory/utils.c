void memset(void* start, int size, int value){

	unsigned char* ptr = (unsigned char*)start;

	for(int i = 0; i < size; i++){

		ptr[i] = (unsigned char)value; 
	}
}