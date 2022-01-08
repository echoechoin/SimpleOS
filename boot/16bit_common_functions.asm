;=============================================================================

; 参数为ax: 字符串的首地址
print_nl:
    pusha
    mov bx, eol
    call print
    popa
    ret

;=============================================================================
print:
    pusha
print_init:
    mov si, bx
    mov ah, 0x0e
    mov bh, 0x00
    mov bl, 0x0f
print_loop:
    mov al, byte [si]
    cmp al, 0x00
    je print_return
    int 0x10
    add si, 1
    jmp print_loop
print_return:
    popa
    ret

;=============================================================================
; 参数为dx: 待打印的字符
print_hex:
    pusha

    mov cx, 0 ; our index variable

; Strategy: get the last char of 'dx', then convert to ASCII
; Numeric ASCII values: '0' (ASCII 0x30) to '9' (0x39), so just add 0x30 to byte N.
; For alphabetic characters A-F: 'A' (ASCII 0x41) to 'F' (0x46) we'll add 0x40
; Then, move the ASCII byte to the correct position on the resulting string
hex_loop:
    cmp cx, 4 ; loop 4 times
    je hex_loop_end
    
    ; 1. convert last char of 'dx' to ascii
    mov ax, dx ; we will use 'ax' as our working register
    and ax, 0x000f ; 0x1234 -> 0x0004 by masking first three to zeros
    add al, 0x30 ; add 0x30 to N to convert it to ASCII "N"
    cmp al, 0x39 ; if > 9, add extra 8 to represent 'A' to 'F'
    jle hex_loop_step2
    add al, 7 ; 'A' is ASCII 65 instead of 58, so 65-58=7

hex_loop_step2:
    ; 2. get the correct position of the string to place our ASCII char
    ; bx <- base address + string length - index of char
    mov bx, HEX_OUT + 5 ; base + length
    sub bx, cx  ; our index variable
    mov [bx], al ; copy the ASCII char on 'al' to the position pointed by 'bx'
    ror dx, 4 ; 0x1234 -> 0x4123 -> 0x3412 -> 0x2341 -> 0x1234

    ; increment index and loop
    add cx, 1
    jmp hex_loop

hex_loop_end:
    ; prepare the parameter and call the function
    ; remember that print receives parameters in 'bx'
    mov bx, HEX_OUT
    call print

    popa
    ret

;=============================================================================
; 作用为加载一个扇区的512字节到内存中
; 参数为ch-dh-cl CHS; es:bx: 内存地址
disk_load:
    pusha
    ; reload disk
    mov ah, 0x00
    int 0x13
    jc disk_load_error
disk_load_init:
    mov ah, 0x02
    mov al, 1
    mov dl, 0
    int 0x13
    jc disk_load_error
    
disk_load_return:
    popa
    ret

disk_load_error:
    mov bx, msg3
    call print
    jmp $

;=============================================================================
; 设置VGA显卡模式
; 参数为al: 模式
set_VGA_MODE:
    pusha
    mov ah, 0x00
    int 0x10
    popa
    ret


msg1:
    db ">> Hello, SimleOS!",            0x0a, 0x0d, 0x00
msg3:
    db ">> load error!",                0x0a, 0x0d, 0x00
eol:
    db 0x0a, 0x0d, 0x00
HEX_OUT:
    db "0x", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

