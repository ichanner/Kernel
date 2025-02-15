BOOTLOADER = ./boot/mbr.asm
BOOTLOADER_BIN = ./out/boot.bin
KERNEL = ./kernel/kernel.c
KERNEL_OBJECT = ./out/kernel.o
KERNEL_ENTRY = ./kernel/kernel_entry.asm
KERNEL_ENTRY_OBJECT = ./out/kernel_entry.o
LINKED_KERNEL = ./out/kernel.bin
FINAL_IMAGE = ./out/os_image.img


INTERRUPT_HANDLERS="./kernel/scheduler.c"
INTERRUPT_HANDLERS_OBJECT="./out/scheduler.o"

build: $(BOOTLOADER) $(KERNEL) $(KERNEL_ENTRY)

	i386-elf-gcc -ffreestanding -m32 -g -c $(KERNEL) -o $(KERNEL_OBJECT)
	nasm -f elf $(KERNEL_ENTRY) -o $(KERNEL_ENTRY_OBJECT)
	
	i386-elf-ld -m elf_i386 -Tlinker.ld -o $(LINKED_KERNEL) $(KERNEL_ENTRY_OBJECT) $(KERNEL_OBJECT) --oformat binary

	nasm -f bin $(BOOTLOADER) -o $(BOOTLOADER_BIN)
	
	dd if=/dev/zero of=$(FINAL_IMAGE) bs=512 count=50
	dd if=$(BOOTLOADER_BIN) of=$(FINAL_IMAGE) conv=notrunc
	dd if=$(LINKED_KERNEL) of=$(FINAL_IMAGE) bs=512 seek=1 conv=notrunc

clean:

	rm -f $(BOOTLOADER_BIN) $(KERNEL_OBJECT) $(KERNEL_ENTRY_OBJECT) $(LINKED_KERNEL) $(FINAL_IMAGE)

