AS   := nasm
DASM := ndisasm
CC   := i386-elf-gcc
LD   := i386-elf-ld
GDB  := i386-elf-gdb

IMG := os-image.bin

C_SOURCES := $(wildcard kernel/*.c drivers/*.c)
HEADERS := $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o}

all: ${IMG}

${IMG}: boot/ipl.bin boot/boot.bin kernel.bin
	dd if=/dev/zero     of=$@ bs=512 count=2880
	dd if=boot/ipl.bin  of=$@ bs=512 count=1 conv=notrunc
	sudo mount $@ mnt
	cat boot/boot.bin kernel.bin > kernel.sys
	sudo cp kernel.sys mnt/
	sudo umount mnt

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext 0x280000 --oformat binary -o $@ $^

%.o: %.asm
	$(AS) -f elf -o $@ $<

%.bin: %.asm
	$(AS) -f bin  -o $@ $<

clean:
	rm -rf kernel.bin kernel.elf ${IMG}
	rm -rf kernel/*.o boot/*.bin boot/*.o
	rm -rf drivers/*.o
