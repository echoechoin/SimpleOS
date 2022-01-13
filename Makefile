AS   := nasm
DASM := ndisasm
CC   := i386-elf-gcc
LD   := i386-elf-ld
GDB  := i386-elf-gdb
QEMU := qemu-system-i386

IMG           := os-image.bin
C_SOURCES     := $(wildcard kernel/*.c drivers/*.c libc/**/*.c)
OBJ           := ${C_SOURCES:.c=.o} kernel/int_handler.o
KERNEL_OFFSET := 0x280000
CFLAGS        := -std=gnu99 -g -I./include -I./libc/include

.PHONY: all clean debug kernel.bin kernel.elf ${IMG}

all: ${IMG}

${IMG}: boot/ipl.bin boot/boot.bin kernel.bin
	dd if=/dev/zero     of=$@ bs=512 count=2880           > /dev/null 2>&1
	dd if=boot/ipl.bin  of=$@ bs=512 count=1 conv=notrunc > /dev/null 2>&1
	cat boot/boot.bin kernel.bin > os-image.elf
	sudo mount $@ mnt
	sudo mv os-image.elf mnt
	sudo umount mnt

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -T kernel.ld --oformat binary $^ -o $@
	# $(LD) -Ttext 0x280000 --oformat binary $^ -o $@

kernel.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext ${KERNEL_OFFSET} $^ -o $@ 

# not working currently
debug:${IMG} kernel.elf
	${QEMU} -gdb tcp::1234 -S -fda ${IMG} &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.asm
	$(AS) -f elf $< -o $@

%.bin: %.asm
	$(AS) -f bin $< -o $@

%.o: %.c
	$(CC) ${CFLAGS} -ffreestanding -c $< -o $@

clean:
	rm -rf *.sys *.bin *.elf ${IMG}
	rm -rf kernel/*.o boot/*.bin boot/*.o
	rm -rf drivers/*.o
	rm -rf **/**/*.o