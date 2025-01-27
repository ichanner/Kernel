#include <stdbool.h>
#include "./screen.c"
#include "./interrupts/interrupt_handlers.c"

void main() {

	disable_interrupts();

	char* msg = "Welcome to Channer OS\0";

	print(msg);	
	println();

	enable_interrupts();


    while (1) { }
}
 




