void memset(void* start, int size, int value){

	unsigned char* ptr = (unsigned char*)start;

	for(int i = 0; i < size; i++){

		ptr[i] = (unsigned char)value; 
	}
}


void memcpy(void* src, void* dest, int size){

	/*
	unsigned char* src = (unsigned char*)src;
	unsigned char* dest = (unsigned char*)dest;

	for(int i = 0; i < size; i++){

		dest[i] = src[i]; 
	}
	*/
}