[org 0x7c00] ; bootloader offset
KERNEL_OFFSET equ 0x8200
init:
    mov bp, 0x9000 ; set the stack
    mov sp, bp 
    mov ax, 0x1000
    mov ss, ax

entry:
    mov bx, msg1
    call print

    mov bx, MSG_REAL_MODE
    call print ; This will be written after the BIOS messages

    mov ax, 0x13
    call set_VGA_MODE
    
    call load_kernel
    call switch_to_pm
    
    jmp $ ; this will actually never be executed

load_kernel:
    pusha
    ; 2. 读取1个柱面的内容到KERNEL_OFFSET
    mov ax, KERNEL_OFFSET
    shr ax, 4

    mov ch, 0x00 ; C0
    mov dh, 0x00 ; H0
    mov cl, 0x02 ; S2
read_loop:
    mov es, ax
    mov bx, 0x0000
    call disk_load

    add ax, 0x20

    add cl, 1
    cmp cl, 18
    jbe read_loop

    mov cl, 0x01
    add dh, 1
    cmp dh, 2
    jb read_loop

load_kernel_end:
    popa

%include "boot/16bit_common_functions.asm"
%include "boot/32bit_gdt.asm"
%include "boot/32bit_switch.asm"

; To jump into 32-bit mode:
;       1. Disable interrupts
;       2. Load our GDT
;       3. Set a bit on the CPU control register cr0
;       4. Flush the CPU pipeline by issuing a carefully crafted far jump
;       5. Update all the segment registers
;       6. Update the stack
;       7. Call to a well-known label which contains the first useful code in 32 bits

[bits 32]
BEGIN_PM: ; after the switch we will get here
   
    call KERNEL_OFFSET

    jmp $

MSG_REAL_MODE db "Started in 16-bit real mode", 0x0a, 0x0d, 0x00

; bootsector
times 510-($-$$) db 0
dw 0xaa55
