[bits 16]
[extern main]
[extern isr_0]
[extern isr_1]
[extern isr_2]
[extern isr_3]
[extern isr_4]
[extern isr_5]
[extern isr_6]
[extern isr_7]
[extern isr_8]
[extern isr_9]
[extern isr_10]
[extern isr_11]
[extern isr_12]
[extern isr_13]
[extern isr_14]
[extern isr_15]
[extern isr_16]
[extern isr_17]
[extern isr_18]
[extern isr_19]
[extern isr_20]
[extern isr_21]
[extern isr_22]
[extern isr_23]
[extern isr_24]
[extern isr_25]
[extern isr_26]
[extern isr_27]
[extern isr_28]
[extern isr_29]
[extern isr_30]
[extern isr_31]
[extern irq_33]
[extern irq_34]
[extern irq_35]
[extern irq_36]
[extern irq_37]
[extern irq_38]
[extern irq_39]
[extern irq_40]
[extern irq_41]
[extern irq_42]	
[extern irq_43]
[extern irq_44]
[extern irq_45]
[extern irq_46]
[extern irq_47]
[extern irq_48]

[extern identity_page_directory]

[extern contextSwitch]
[extern scheduleProcess]

[global enable_paging]

call set_video_mode
call load_gdt
call enter_protected_mode
call init_pic
call load_idt
call load_tss_register

jmp CODE_SEG:kernel_start

load_tss_register:
	mov ax, 24d
	ltr ax
	ret

set_video_mode:
  mov ah, 0x0 ; service for setting the video mode
  mov al, 0x03 ; setting video mode to be text mode w/ 16 colors
  int 0x10
  ret

load_gdt:
  cli ; disable interupts 
  lgdt [gdtr]
  ret
  
enter_protected_mode:
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  ret

init_pic:

  ; initialze the pic by sending 11h to commands port (addressed at 20h for master and a0h for slave pic)

  mov al, 0x11 
  out 0x20, al
  out 0xa0, al

  ; after initializing the pic it will wait for 4 parameters to be written to its data port (21h for master pic and a1h for slave pic)
  ; first parameter will be the new starting offset for the IRQ's interupt numbers for the interupts

  mov al, 32d
  out 0x21, al
  mov al, 40d
  out 0xa1, al ; 32 + 8

  ; second parameter tells us which which slot the other PIC is connected 

  mov al, 0x4 ; bitmask 0000 0010
  out 0x21, al

  mov al, 0x2 ; slave passes irq id directly
  out 0xa1, al

  ; third parameter tells us which mode the PIC is running on (x86 mode)

  mov al, 0x01
  out 0x21, al
  out 0xa1, al

  ; fourth parameter tells us which IRQ's to enable and which to disable 

  mov al, 0x0 ; bitmask to enable all
  out 0x21, al
  out 0xa1, al

  ret

load_idt:

  lidt [idtr]

  ret



[bits 32]

enable_paging: 
  
  mov eax, [identity_page_directory]
  mov cr3, eax

  mov eax, cr0
  or eax, 0x80000000 ; 31st bit set to 1
  mov cr0, eax

 ret

kernel_start: 
	
	mov eax, DATA_SEG
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov fs, eax
	mov gs, eax

	mov ebp, 0xF000 ; 0x8200 + 3072 bytes
	mov esp, ebp

	sti

	call main

%include './kernel/gdt.asm'
%include './kernel/interrupts/idt.asm'

CODE_SEG equ kernel_code-gdt
DATA_SEG equ kernel_data-gdt



tss: 
    dd 0,
    dw 0,
    dw 0,
    dw 0,
    dd 0,
    dw 0,
    dw 0,
    dd 0,
    dw 0,
    dw 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dd 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    dw 0,
    db 0,
    dw 0,
    db 0,
    dw 0



