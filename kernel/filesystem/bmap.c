void syncBitmap(drive_t drive, int partition_index){

	superblock_t* superblock = &drive.partitions[partition_index];

	if(superblock == NULL) return;

	drive.write(superblock->bitmap_sector_count, superblock->bitmap_start_sector, superblock->sector_bitmap, drive);
}

void setSectors(int sector_start_index, int sector_end_index, int present, int* sector_bitmap){

	for(int i = sector_start_index; i < sector_end_index; i++){

		int remainder = i % 32;
    	int entry = sector_bitmap[i/32];

	    if (present == 1){
    
	        sector_bitmap[i/32] = entry | (1 << remainder);
	    }
	    else{
	        
	        sector_bitmap[i/32] = entry &  ~(1 << remainder);
	    }
	}
}

int getSector(int sector_index, int* sector_bitmap){

	int remainder = sector_index % 32;
    int sector = (sector_bitmap[sector_index/32] >> remainder) & 0x1;
    
    return sector;
}	


int getFreeRegionExtent(int curr_ptr, const int limit, int* sector_bitmap) {

	int free_sectors = 1;
	int free_sector_index = curr_ptr + 1;

	while(getSector(free_sector_index, sector_bitmap) != 1 && free_sector_index < limit) {

		free_sectors++;
		free_sector_index++;
	}

	return free_sectors;
}

int findContinousFreeRegion(int sectors_needed, int start_ptr, const int limit, int* sector_bitmap) {

	int curr_ptr = start_ptr; // sector index tracker
	int best_fit_ptr = -1;
	int best_fit_free_region = -1;

	// if the curr sector index is beyond physical capacity stop looking
	while(curr_ptr < limit) { 	

		if(getSector(curr_ptr, sector_bitmap) == 0){

			// when a free sector is found, try to see how far this free region extend
			int free_region = getFreeRegionExtent(curr_ptr, limit, sector_bitmap);

			// if this free extent can fit what we want and it's smaller than best_fit_free_region then update
			if(free_region >= sectors_needed && (best_fit_free_region < 0 || best_fit_free_region > free_region)){

				best_fit_free_region = free_region;
				best_fit_ptr = curr_ptr;
			}
		
			curr_ptr += free_region;
		}
		else{

			curr_ptr += 1; 
		}
	}

	return best_fit_ptr;
	
}