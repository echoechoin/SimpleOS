#ifndef _PORT_H_
#define _PORT_H_

unsigned char port_byte_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
unsigned short port_word_in (unsigned short port);
void port_word_out (unsigned short port, unsigned short data);

void _io_cli (void);
void _io_sti (void);
void _io_hlt (void);
void _io_stihlt (void); // sti和hlt会合并成一个指令，不会被中断打断



unsigned int _io_load_eflags();
void _io_restore_eflags (unsigned int eflags);

int _io_load_cr0(void);
void _io_restore_cr0(int cr0);

#endif