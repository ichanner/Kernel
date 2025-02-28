

isr_14:
	
	cli

	;push eax
 	
 	mov eax, [esp]
 	
 	push eax

	mov eax, cr2

	push eax

	call handlePageFault 

	mov eax, cr2
	
	invlpg [eax]

	add esp, 12d

;	pop eax

	sti


	iret 

irq_32:

	cli

   	push esi

   	mov esi, [esp + 4]
   	push esi

   	push edi
   	push edx
   	push ecx 
   	push ebx
   	push eax

    call scheduleProcess  

    add esp, 28d

    mov al, 0x20
    out 0x20, al

;    call calculateLRU

    call contextSwitch

    iret


; [MSB Offset (Byte 6 & 7)] [P | DPL | 0D11T | 0000 | Resverd (byte 4 and 5)]  [Segment Selector (Byte 3 & 2)][LSB Offset (Byte 1 and 0)] 

idt:

	isr_0_entry:
	    dw isr_0,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_1_entry:
	    dw isr_1,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_2_entry:
	    dw isr_2,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_3_entry:
	    dw isr_3,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_4_entry:
	    dw isr_4,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_5_entry:
	    dw isr_5,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_6_entry:
	    dw isr_6,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_7_entry:
	    dw isr_7,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_8_entry:
	    dw isr_8,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_9_entry:
	    dw isr_9,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_10_entry:
	    dw isr_10,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_11_entry:
	    dw isr_11,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_12_entry:
	    dw isr_12,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_13_entry:
	    dw isr_13,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_14_entry:
	    dw isr_14,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_15_entry:
	    dw isr_15,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_16_entry:
	    dw isr_16,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_17_entry:
	    dw isr_17,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_18_entry:
	    dw isr_18,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_19_entry:
	    dw isr_19,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_20_entry:
	    dw isr_20,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_21_entry:
	    dw isr_21,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_22_entry:
	    dw isr_22,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_23_entry:
	    dw isr_23,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_24_entry:
	    dw isr_24,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_25_entry:
	    dw isr_25,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_26_entry:
	    dw isr_26,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_27_entry:
	    dw isr_27,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_28_entry:
	    dw isr_28,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_29_entry:
	    dw isr_29,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_30_entry:
	    dw isr_30,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_31_entry:
	    dw isr_31,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_32_entry:
	    dw irq_32,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_33_entry:
	    dw irq_33,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_34_entry:
	    dw irq_34,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_35_entry:
	    dw irq_35,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_36_entry:
	    dw irq_36,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_37_entry:
	    dw irq_37,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_38_entry:
	    dw irq_38,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_39_entry:
	    dw irq_39,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_40_entry:
	    dw irq_40,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_41_entry:
	    dw irq_41,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_42_entry:
	    dw irq_42,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_43_entry:
	    dw irq_43,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_44_entry:
	    dw irq_44,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_45_entry:
	    dw irq_45,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_46_entry:
	    dw irq_46,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 a

	isr_47_entry:
	    dw irq_47,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

	isr_48_entry:
	    dw irq_48,
	    dw 0x8,
	    db 0x00,
	    db 10001110b,
	    dw 0x0 

idt_end:

; [base address of IDT [47:16] ][size of IDT [15:0] ]

idtr:

	idt_size: dw idt_end - idt 
	idt_base: dd idt 

