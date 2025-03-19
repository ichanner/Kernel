

void setConfigAddress(unsigned int bus, unsigned int slot, unsigned int func, unsigned int offset){

	outl((unsigned int)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC ) | 0x80000000), 0xCF8);
}

unsigned int pciConfigReadDWord(unsigned int bus, unsigned int slot, unsigned int func, unsigned int offset) {
    
   	setConfigAddress(bus, slot, func, offset);
    
    return inl(0xCFC);
}


void pciConfigWriteDword(unsigned int bus, unsigned int slot, unsigned int func, unsigned int offset, unsigned int data){

	setConfigAddress(bus, slot, func, offset);

	outl(data, 0xCFC);
}


//maybe by device id or vendor id... idk..
void enumeratePCI(unsigned int class_code, unsigned int subclass, unsigned int* dev_bus, unsigned int* dev_slot, unsigned int* dev_func){

	for(int bus = 0; bus < 255; bus++) {

		for(int slot = 0; slot < 32; slot++) {

			for(int func = 0; func < 8; func++){

				unsigned int config = pciConfigReadDWord(bus, slot, func, 0x8);
				unsigned int config_subclass = (config >> 16) & 0xFF;
				unsigned int config_class = (config >> 24) & 0xFF;

				if(config_subclass == subclass && config_class == class_code){

					*dev_bus = bus;
					*dev_slot = slot;
					*dev_func = func;

					return;
				}			

			}
		}
	}
}