#include "../drivers/utils/port_io.c"
#include "../drivers/char/keyboard.c"
#include "../proc/scheduler.c"
#include <stdbool.h>

void enable_interrupts(){

	asm volatile("sti");
}


void disable_interrupts(){

	asm volatile("cli");
}

void halt(){

	asm volatile("hlt");
}

void handle_critical_error(const char* error_message) {

	disable_interrupts();
	print(error_message);
	halt();
}

void irq_cleanup(bool is_slave_pic){
	
	if(is_slave_pic == true){ //slave pic EOI

		outb(0xa0, 0xa0);
	}
	else { // master pic EOI  

		outb(0x20, 0x20);
	}

	enable_interrupts();

}


void isr_0() { 

	handle_critical_error("Divide by Zero Error\n");

}
void isr_1() { 

	print("Debug Exception\n"); 

}
void isr_2() { 

	print("Non-Maskable Interrupt\n"); 
	halt();

}
void isr_3() {  

	print("Breakpoint Exception\n");

 }
void isr_4() {  

	print("Overflow Exception\n"); 
}
void isr_5() { 

	print("Bound Range Exceeded\n"); 
	halt();
}
void isr_6() { 

	handle_critical_error("Invalid Opcode Exception\n");
}

void isr_7() { 
    print("Device Not Available Exception\n"); 
    halt();
}

void isr_8() { 
    handle_critical_error("Double Fault\n");
}

void isr_9() { 
    print("Coprocessor Segment Overrun (Legacy)\n"); 
    halt();
}

void isr_10() { 
    print("Invalid TSS (Task State Segment)\n"); 
    halt();
}

void isr_11() { 
    print("Segment Not Present\n"); 
    halt();
}

void isr_12() { 
    print("Stack-Segment Fault\n"); 
    halt();
}

void isr_13() { 
    handle_critical_error("General Protection Fault\n");
}

/*
void isr_14() { 

    print("Page Fault\n");
}
*/

void isr_15() { 
    print("Reserved Interrupt (15)\n"); 
    halt();
}

void isr_16() { 
    print("x87 Floating-Point Error\n"); 
    halt();
}

void isr_17() { 
    print("Alignment Check Exception\n"); 
    halt();
}

void isr_18() { 
    handle_critical_error("Machine Check Exception\n");
}

void isr_19() { 
    print("SIMD Floating-Point Exception\n"); 
    halt();
}

// Reserved ISRs (20-31)
void isr_20() { print("Reserved ISR 20\n"); halt(); }
void isr_21() { print("Reserved ISR 21\n"); halt(); }
void isr_22() { print("Reserved ISR 22\n"); halt(); }
void isr_23() { print("Reserved ISR 23\n"); halt(); }
void isr_24() { print("Reserved ISR 24\n"); halt(); }
void isr_25() { print("Reserved ISR 25\n"); halt(); }
void isr_26() { print("Reserved ISR 26\n"); halt(); }
void isr_27() { print("Reserved ISR 27\n"); halt(); }
void isr_28() { print("Reserved ISR 28\n"); halt(); }
void isr_29() { print("Reserved ISR 29\n"); halt(); }
void isr_30() { print("Reserved ISR 30\n"); halt(); }
void isr_31() { print("Reserved ISR 31\n"); halt(); }

void irq_33() { 

	disable_interrupts();

	int status = inb(0x64);

	if(status | 1 == 1) {

		int buffer = inb(0x60);

		onKeyPressed(buffer);
		onKeyReleased(buffer);
		
		char test[] = {codeToAscii(buffer), '\0'};
		
		print(test);

	}


	irq_cleanup(false);

}
void irq_34() { 

	disable_interrupts();

	print("IRQ: 34\n"); 

	irq_cleanup(false);
}
void irq_35() { 

	disable_interrupts();

	print("IRQ: 35\n"); 

	irq_cleanup(false);

}
void irq_36() { 

	disable_interrupts();

	print("IRQ: 36\n"); 

	irq_cleanup(false);
}
void irq_37() { 

	disable_interrupts();

	print("IRQ: 37\n");

	irq_cleanup(false);
}


void irq_38() { 

	disable_interrupts();

	print("IRQ: 38\n"); 

	irq_cleanup(false);

}
void irq_39() { 


	disable_interrupts();

	print("IRQ: 39\n"); 

	irq_cleanup(false);
}
void irq_40() { 

	disable_interrupts();

	print("IRQ: 40\n"); 

	irq_cleanup(true);

}

void irq_41() { 

	disable_interrupts();

	print("IRQ: 41\n"); 

	irq_cleanup(true);
}
void irq_42() { 

	disable_interrupts();

	print("IRQ: 42\n"); 

	irq_cleanup(true);
}
void irq_43() { 

	disable_interrupts();

	print("IRQ: 43\n"); 

	irq_cleanup(true);
}
void irq_44() { 

	disable_interrupts();

	print("IRQ: 44\n"); 

	irq_cleanup(true);
}
void irq_45() { 

	disable_interrupts();

	print("IRQ: 45\n"); 

	irq_cleanup(true);
}


void irq_46() { 

	disable_interrupts();

	print("IRQ: 46\n"); 

	irq_cleanup(true);

}
void irq_47() { 

	disable_interrupts();

	print("IRQ: 47\n"); 

	irq_cleanup(true);

}
void irq_48() { 

	disable_interrupts();

	print("IRQ: 48\n"); 

	irq_cleanup(true);
}