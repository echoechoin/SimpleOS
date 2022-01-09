;=============================================================================
; 打印行结束符
print_nl:
    pusha
    mov bx, MSG_EOL
    call print
    popa
    ret

;=============================================================================
; 打印字符串
; 参数为ax: 字符串的首地址
print:
    pusha
__print_init:
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

;=============================================================================
; 打印dx中的内容为16进制
print_hex:
    pusha
    mov cx, 0 

__hex_loop:
    cmp cx, 4
    je __hex_loop_end

    mov ax, dx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39
    jle __hex_loop_step2
    add al, 7

__hex_loop_step2:
    mov bx, HEX_OUT + 5
    sub bx, cx
    mov [bx], al
    ror dx, 4
    add cx, 1
    jmp __hex_loop

__hex_loop_end:
    mov bx, HEX_OUT
    call print

    popa
    ret

;=============================================================================
; 加载内核
load_kernel:
    pusha
    mov ax, KERNEL_OFFSET
    shr ax, 4

    mov ch, 0x00 ; C0
    mov dh, 0x00 ; H0
    mov cl, 0x02 ; S2
    mov es, ax
    mov bx, 0x0000

__read_loop:
    ; ; 调试用
    ; push dx
    ; mov dx, es
    ; call print_hex
    ; call print_nl
    ; pop dx

    MOV ah, 0x02          ; AH=0x02：读盘
    MOV al, 1             ; 1个扇区
    MOV dl, 0x00          ; A驱动器
    INT 0x13              ; 调用磁盘BIOS
    jc __read_error

    mov ax, es
    add ax, 0x0020
    mov es, ax

    add cl, 1
    cmp cl, 18
    jbe __read_loop

    mov cl, 1
    add dh, 1
    cmp dh, 2
    jb __read_loop

    mov dh, 0
    add ch, 1
    cmp ch, 10
    jb __read_loop

__load_kernel_end:
    popa
    ret


__read_error:
    mov ax, MSG_LOAD_ERR
    call print
    jmp $

;=============================================================================
; 设置VGA显卡模式
set_vga_mode:
    pusha
    mov ax, 0x13
    int 0x10
    popa
    ret

;=============================================================================
debug:
    pusha
    mov bx, MSG_DEBUG
    call print
    popa
    ret

MSG_LOGO      db ">> Hello, SimpleOS!",         0x0a, 0x0d, 0x00
MSG_LOAD_ERR  db ">> load error!",              0x0a, 0x0d, 0x00
MSG_DEBUG     db ">> debug",                    0x0a, 0x0d, 0x00
MSG_REAL_MODE db "Started in 16-bit real mode", 0x0a, 0x0d, 0x00
MSG_EOL       db                                0x0a, 0x0d, 0x00
HEX_OUT       db "0x", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

