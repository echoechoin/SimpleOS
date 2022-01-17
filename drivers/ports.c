/**
 * Read a byte from the specified port
 */
unsigned char port_byte_in (unsigned short port) {
    unsigned char result;
    /* Inline assembler syntax
     * !! Notice how the source and destination registers are switched from NASM !!
     *
     * '"=a" (result)'; set '=' the C variable '(result)' to the value of register e'a'x
     * '"d" (port)': map the C variable '(port)' into e'd'x register
     *
     * Inputs and outputs are separated by colons
     */
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out (unsigned short port, unsigned char data) {
    /* Notice how here both registers are mapped to C variables and
     * nothing is returned, thus, no equals '=' in the asm syntax 
     * However we see a comma since there are two variables in the input area
     * and none in the 'return' area
     */
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

unsigned short port_word_in (unsigned short port) {
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out (unsigned short port, unsigned short data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

unsigned int _io_load_eflags() {
    unsigned int eflags;
    __asm__("pushfl");
    __asm__("pop %0" : "=r" (eflags));
    return eflags;
}

void _io_restore_eflags (unsigned int eflags) {
    __asm__("push %0" : : "r" (eflags));
    __asm__("popfl");
}

int _io_load_cr0(void) {
    int cr0;
    __asm__("mov %%cr0, %0" : "=r" (cr0));
}

void _io_restore_cr0(int cr0) {
    __asm__("mov %0, %%cr0" : : "r" (cr0));
}

void _io_cli (void) {
    __asm__("cli");
}

void _io_sti (void) {
    __asm__("sti");
}