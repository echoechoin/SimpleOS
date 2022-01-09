[org 0x7c00] ; bootloader offset
KERNEL_OFFSET equ 0x8000
init:
    MOV ax, 0
    MOV ss, ax
    mov sp, 0x7c00
    mov ds, ax 

entry:
    mov bx, MSG_LOGO
    call print

    mov bx, MSG_REAL_MODE
    call print

    call load_kernel
    call set_vga_mode
    call switch_to_pm

%include "boot/16bit_common_functions.asm"
%include "boot/32bit_gdt.asm"
%include "boot/32bit_switch.asm"

[bits 32]
BEGIN_PM:
    call KERNEL_OFFSET
    jmp $

times 510-($-$$) db 0
dw 0xaa55
