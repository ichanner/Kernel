[org 0x7c00] ; bootloader starts at 0x7c00 
[bits 16]

KERNEL_SECTORS_COUNT equ 0x31 ; 49 sectors to read 
KERNEL_LOCATION equ 0x8200 ; (0x7c00 + 512) + 512 + 512 

xor ax, ax
mov es, ax
mov ds, ax

mov bp, 0x8000 ; (0x7c00 + 512) + 512 
mov sp, bp

mov ah, 0x2
mov al, KERNEL_SECTORS_COUNT 
mov ch, 0x0 ; starting cylinder 
mov cl, 0x2 ; starting sector
mov dh, 0x0 ; starting head
mov bx, KERNEL_LOCATION
int 0x13

jc kernel_load_error ; carry flag set on error

mov ebx, 0
mov di, 0x8000
mov edx, 0

jmp KERNEL_LOCATION

kernel_load_error:
	mov bx, 0
	jmp print_kernel_error

print_kernel_error:
	mov al, [kernel_error_message + bx]
	mov ah, 0x0e
	cmp al, 0
	je exit_kernel_error
	inc bx
	jmp print_kernel_error

exit_kernel_error:
	jmp $

kernel_error_message: db "Unable to load kernel image", 0

times 494 - ($-$$) db 0 

partion_table:
	
	db 0x80                  ; Status
	db 0x00                  ; Starting LBA address 
	db 0x07                  ; Partition type
	db KERNEL_SECTORS_COUNT  ; Partition Size in sectors

	dd 0, ; Partiton 2
	dd 0, ; Partiton 3
	dd 0  ; Partiton 4

db 0x55, 0xaa


	
