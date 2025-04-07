void* allocBuffer(unsigned int size, drive_t drive) {

	if(drive.dma_mode != -1) {

		void* buffer = allocPages(size);

		identityMapPages(buffer, size, 1, 0, 0, 1);

		return buffer;
	}

	return kalloc(size);
}


int allocMetaSector(superblock_t* superblock) {


	int curr_ptr = superblock->meta_region; 

	while(curr_ptr < superblock->data_region){

		if(getSector(curr_ptr, superblock->sector_bitmap) == 0) {

			setSectors(curr_ptr, curr_ptr + 1, 1, superblock->sector_bitmap);

			return curr_ptr;
		}

		curr_ptr += 1;
	}	

	return -1;
}
