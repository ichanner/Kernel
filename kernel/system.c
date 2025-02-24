//dest, src


void outb(unsigned char value,  unsigned short port){

	asm volatile("outb %0, %1\n" : : "a"(value), "Nd"(port));
}

void outw(unsigned short value,  unsigned short port){

	asm volatile("outw %0, %1\n" : : "a"(value), "Nd"(port));
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



void enable_interrupts(){

	asm volatile("sti");
}


void disable_interrupts(){

	asm volatile("cli");
}

void halt(){

	asm volatile("hlt");
}
