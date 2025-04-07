void memset(void* start, int size, int value){

	unsigned char* ptr = (unsigned char*)start;

	for(int i = 0; i < size; i++){

		ptr[i] = (unsigned char)value; 
	}
}


void memcpy(void* src, void* dest, int size){

	char* src_cast = (char*)src;
	char* dest_cast = (char*)dest;

	for(int i = 0; i < size; i++){

		dest_cast[i] = src_cast[i]; 
	}

}