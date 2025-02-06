//dest, src

void outb(unsigned char value,  unsigned short port){

	asm volatile("outb %0, %1\n" : : "a"(value), "Nd"(port));
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
