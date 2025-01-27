[bits 16]
[extern main]


call set_video_mode
call load_gdt
call enter_protected_mode
call init_pic
call load_idt

jmp CODE_SEG:kernel_start

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

kernel_start: 
	
	mov eax, DATA_SEG
	mov ds, eax
	mov ss, eax
	mov es, eax
	mov fs, eax
	mov gs, eax

	mov ebp, 0x9c00 ; 0x8200 + 
	mov esp, ebp

	sti

	call main

%include './kernel/gdt.asm'
%include './kernel/interrupts/idt.asm'
CODE_SEG equ kernel_code-gdt
DATA_SEG equ kernel_data-gdt
