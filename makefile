BOOTLOADER = ./boot/mbr.asm
KERNEL = ./kernel/kernel.c
KERNEL_ENTRY = ./kernel/kernel_entry.asm
SWAP = ./kernel/memory/swap.c
UTILS = ./kernel/memory/utils.c
ALLOC = ./kernel/memory/allocator.c

HEAP_MANAGER = ./kernel/memory/heap_manager.c
PAGING = ./kernel/memory/paging.c
FRAME_MANAGER = ./kernel/memory/frame_manager.c
PAGING_OBJECT = ./out/paging.o
UTILS_OBJECT = ./out/utils.o

SWAP_OBJECT = ./out/swap.o
ALLOC_OBJECT = ./out/allocator.o

FRAME_MANAGER_OBJECT = ./out/frame_manager.o
KERNEL_ENTRY_OBJECT = ./out/kernel_entry.o
KERNEL_OBJECT = ./out/kernel.o
BOOTLOADER_BIN = ./out/boot.bin
LINKED_KERNEL = ./out/kernel.bin
FINAL_IMAGE = ./out/os_image.img
SUM2 = ./user/sum2.c

OBJECTS = $(KERNEL_ENTRY_OBJECT) $(UTILS_OBJECT) $(ALLOC_OBJECT) $(KERNEL_OBJECT) $(PAGING_OBJECT) $(SWAP_OBJECT) $(FRAME_MANAGER_OBJECT)

build: $(BOOTLOADER) $(KERNEL) $(SWAP) $(PAGING) $(FRAME_MANAGER) $(KERNEL_ENTRY)

	i386-elf-gcc -ffreestanding -m32 -g -c -w $(KERNEL) -o $(KERNEL_OBJECT)
	i386-elf-gcc -ffreestanding -m32 -g -c -w $(PAGING) -o $(PAGING_OBJECT)
	i386-elf-gcc -ffreestanding -m32 -g -c -w $(SWAP) -o $(SWAP_OBJECT)
	i386-elf-gcc -ffreestanding -m32 -g -c -w $(UTILS) -o $(UTILS_OBJECT)
	i386-elf-gcc -ffreestanding -m32 -g -c -w $(ALLOC) -o $(ALLOC_OBJECT)
	i386-elf-gcc -ffreestanding -m32 -g -c -w $(FRAME_MANAGER) -o $(FRAME_MANAGER_OBJECT)
	
	nasm -f elf $(KERNEL_ENTRY) -o $(KERNEL_ENTRY_OBJECT)
	
	i386-elf-ld -m elf_i386 -Tlinker.ld -o $(LINKED_KERNEL) $(OBJECTS) --oformat binary

	nasm -f bin $(BOOTLOADER) -o $(BOOTLOADER_BIN)
	
	dd if=/dev/zero of=$(FINAL_IMAGE) bs=512 count=100
	dd if=$(BOOTLOADER_BIN) of=$(FINAL_IMAGE) conv=notrunc
	dd if=$(LINKED_KERNEL) of=$(FINAL_IMAGE) bs=512 seek=1 conv=notrunc
	dd if=$(SUM2) of=$(FINAL_IMAGE) bs=512 seek=50 conv=notrunc

	dd if=$(SUM2) of=$(FINAL_IMAGE) bs=512 seek=51 conv=notrunc


#sudo dd if=$(FINAL_IMAGE) of=/dev/sdb bs=4M status=progress

clean:

	rm -f $(BOOTLOADER_BIN) $(KERNEL_OBJECT) $(KERNEL_ENTRY_OBJECT) $(LINKED_KERNEL) $(FINAL_IMAGE)

