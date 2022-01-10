[org 0x7c00]
LOAD_ADDR     equ 0x8000
KERNEL_OFFSET equ 0xc200
    JMP   entry             ; 跳转指令
    NOP                     ; NOP指令
    DB    "SIMPLEOS"        ; OEM标识符（8字节）
    DW    512               ; 每个扇区（sector）的字节数（必须为512字节）
    DB    1                 ; 每个簇（cluster）的扇区数（必须为1个扇区）
    DW    1                 ; FAT的预留扇区数（包含boot扇区）
    DB    2                 ; FAT表的数量，通常为2
    DW    224               ; 根目录文件的最大值（一般设为224项）
    DW    2880              ; 磁盘的扇区总数，若为0则代表超过65535个扇区，需要使用22行记录
    DB    0xf0              ; 磁盘的种类（本项目中设为0xf0代表1.44MB的软盘）
    DW    9                 ; 每个FAT的长度（必须为9扇区）
    DW    18                ; 1个磁道（track）拥有的扇区数（必须是18）
    DW    2                 ; 磁头数（必须为2）
    DD    0                 ; 隐藏的扇区数
    DD    2880              ; 大容量扇区总数，若16行记录的值为0则使用本行记录扇区数
    DB    0                 ; 中断0x13的设备号
    DB    0                 ; Windows NT标识符
    DB    0x29              ; 扩展引导标识
    DD    0xffffffff        ; 卷序列号
    DB    "SIMPLEOS   "     ; 卷标（11字节）
    DB    "FAT12   "        ; 文件系统类型（8字节）
    RESB  18                ; 空18字节

entry:
    MOV ax, 0
    MOV ss, ax
    mov sp, 0x7c00
    mov ds, ax 
    
    mov bx, MSG_LOGO
    call print
    call load_kernel
    jmp KERNEL_OFFSET

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

load_kernel:
    pusha
    mov bx, MSG_LOAD_START
    call print

    mov ax, LOAD_ADDR
    shr ax, 4

    mov ch, 0x00 ; C0
    mov dh, 0x00 ; H0
    mov cl, 0x02 ; S2
    mov es, ax
    mov bx, 0x0000

__read_loop:
    mov ah, 0x02          ; AH=0x02：读盘
    mov al, 1             ; 1个扇区
    mov dl, 0x00          ; A驱动器
    int 0x13              ; 调用磁盘BIOS
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
    mov bx, MSG_LOAD_SUCCESS
    call print
    popa
    ret

__read_error:
    mov ax, MSG_LOAD_ERR
    call print
    jmp $

MSG_LOGO         db "hello SimpleOS!", 0x0d, 0x0a, 0x00
MSG_LOAD_START   db "Loading kernel...", 0x0d, 0x0a, 0x00
MSG_LOAD_SUCCESS db "Load kernel success!", 0x0d, 0x0a, 0x00
MSG_LOAD_ERR     db "load kernel error!", 0x0d, 0x0a, 0x00

times 510-($-$$) db 0
dw 0xaa55
  