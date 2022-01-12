[bits 32]

global asm_int_handler21, asm_int_handler27, asm_int_handler2c

extern int_handler21, int_handler27, int_handler2c

asm_int_handler21:
    push es
    push ds
    pushad
    mov eax, esp
    push eax
    mov ax, ss
    mov ds, ax
    mov es, ax
    call int_handler21
    pop eax
    popad
    pop ds
    pop es
    iretd

asm_int_handler27:
    push es
    push ds
    pushad
    mov eax, esp
    push eax
    mov ax, ss
    mov ds, ax
    mov es, ax
    call int_handler27
    pop eax
    popad
    pop ds
    pop es
    iretd

asm_int_handler2c:
    push es
    push ds
    pushad
    mov eax, esp
    push eax
    mov  ax, ss
    mov  ds, ax
    mov  es, ax
    call int_handler2c
    pop eax
    popad
    pop ds
    pop es
    iretd
