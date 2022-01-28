[org 0xc200]
[bits 16]

KERNEL_ENTRY EQU   0x00280000
DSKCAC       EQU   0x00100000
DSKCAC0      EQU   0x00008000

CYLS         EQU   0x0ff0      ; 设置启动区
LEDS         EQU   0x0ff1
VMODE        EQU   0x0ff2      ; 关于颜色数目的信息，颜色的位数
SCRNX        EQU   0x0ff4      ; 分辨率X
SCRNY        EQU   0x0ff6      ; 分辨率Y
VRAM         EQU   0x0ff8      ; 图像缓冲区的起始位置


entry:
    call set_vga_mode_320x200x8
    call enable_a20_gate
    call switch_to_32bit_mode
    jmp $

set_vga_mode_320x200x8:
    pusha
    mov ax, 0x13
    int 0x10
    mov byte  [VMODE], 8
    mov word  [SCRNX], 320
    mov word  [SCRNY], 200
    mov dword  [VRAM],  0x000a0000
    popa
    ret

enable_a20_gate:
    call waitkbdout
    mov al, 0xd1
    out 0x64, al
    call waitkbdout
    mov al, 0xdf          ; 开启a20
    out 0x60, al
    call waitkbdout
    ret

waitkbdout:
    in al, 0x64
    and al, 0x02
    jnz waitkbdout         ; and结果不为0跳转至waitkbdout
    ret

print:
    pusha
    mov si, bx
    mov ah, 0x0e
    mov bh, 0x00
    mov bl, 0x0f
__print_loop:
    mov al, byte [si]
    cmp al, 0x00
    je __print_return
    int 0x10
    add si, 1
    jmp __print_loop
__print_return:
    popa
    ret

switch_to_32bit_mode:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp init_pm

init_pm:
    mov ax, DATA_SEG 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; kernel传递
    mov esi, kernel; 源
    mov edi, KERNEL_ENTRY; 目标
    mov ecx, 512*1024/4
    call memcpy

skip:
    mov esp, 0xffff
    JMP DWORD CODE_SEG:0

memcpy:
    mov eax, [esi]
    add esi, 4
    mov [edi], eax
    add edi, 4
    sub ecx, 1
    jnz memcpy            ; 结果不为0跳转至memcpy
    ret

align 16
GDT_START resb  8
GDT_DATA  dw 0xffff, 0x0000, 0x9200, 0x00cf  ; 可写的32位段寄存器
GDT_CODE  dw 0xffff, 0x0000, 0x9a28, 0x0047  ; 可执行的文件的32位寄存器
          dw 0
GDT_END:
gdt_descriptor:
    dw GDT_END - GDT_START - 1
    dd GDT_START
CODE_SEG equ GDT_CODE - GDT_START
DATA_SEG equ GDT_DATA - GDT_START

kernel:
