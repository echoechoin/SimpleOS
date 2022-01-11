unsigned char port_byte_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
unsigned short port_word_in (unsigned short port);
void port_word_out (unsigned short port, unsigned short data);

void _io_cli (void);
void _io_sti (void);


unsigned int _io_load_eflags();
void _io_restore_eflags (unsigned int eflags);
