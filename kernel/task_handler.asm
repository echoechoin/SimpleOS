[bits 32]

    global load_tr
    global far_jmp, task_switch4, task_switch3

load_tr:            ; void load_tr(int tr);
    ltr   [esp+4]     ; tr
    ret

far_jmp:            ; void far_jmp(int eip, int cs);
    jmp   far [esp+4]
    ret
