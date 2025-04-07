//dest, src

void outl(unsigned int value,  unsigned short port){

	asm volatile("outl %0, %1\n" : : "a"(value), "Nd"(port));
}


void outb(unsigned char value,  unsigned short port){

	asm volatile("outb %0, %1\n" : : "a"(value), "Nd"(port));
}

void outw(unsigned short value,  unsigned short port){

	asm volatile("outw %0, %1\n" : : "a"(value), "Nd"(port));
} 

unsigned int inl(unsigned short port){

	unsigned int buffer;

	asm volatile("inl %1, %0\n" : "=r"(buffer) : "Nd"(port));

	return buffer;
}

unsigned short inw(unsigned short port){

	unsigned short buffer;

	asm volatile("inw %1, %0\n" : "=r"(buffer) : "Nd"(port));

	return buffer;
}

unsigned char inb(unsigned short port){

	unsigned char buffer;

	asm volatile("inb %1, %0\n" : "=r"(buffer) : "Nd"(port));

	return buffer;
}
