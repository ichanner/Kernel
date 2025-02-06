
gdt:

    null_descriptor: db 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0

    kernel_code: 

        db 11111111b, ; lsb limit
        db 11111111b, ; lsb limit
        db 0x0, ; lsb base addr
        db 0x0, ; lsb base addr 
        db 0x0, ; lsb base addr
        db 10011010b,  ; [ P | DPL[1:0] | S | Type[3:0]]
        db 11001111b,  ; [ G | D/B | L | AVL | MSB limit[19:16]]
        db 0x0 ; msb base addr

    kernel_data: 

        db 11111111b, ; lsb limit
        db 11111111b, ; lsb limit
        db 0x0, ; lsb base addr
        db 0x0, ; lsb base addr
        db 0x0, ; lsb base addr
        db 10010010b ; [ P | DPL[1:0] | S | Type[3:0]]
        db 11001111b, ; [ G | D/B | L | AVL | MSB limit[19:16]]
        db 0x0 ; msb base addr

    tss_descriptor: dw 103d, tss, 1000100100000000b, 0x0

gdt_end:

gdtr:
    size: dw gdt_end - gdt - 1
    base: dd gdt