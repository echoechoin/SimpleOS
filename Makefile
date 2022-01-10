AS   := nasm
DASM := ndisasm
CC   := i386-elf-gcc
LD   := i386-elf-ld
GDB  := i386-elf-gdb
QEMU := qemu-system-i386

IMG           := os-image.bin
C_SOURCES     := $(wildcard kernel/*.c drivers/*.c)
HEADERS       := $(wildcard kernel/*.h drivers/*.h)
OBJ           := ${C_SOURCES:.c=.o}
KERNEL_OFFSET := 0x280000
CFLAGS        := -g

all: ${IMG}

${IMG}: boot/ipl.bin boot/boot.bin kernel.bin
	@echo "Create image..."
	@dd if=/dev/zero     of=$@ bs=512 count=2880           > /dev/null 2>&1
	@echo "load IPL..."
	@dd if=boot/ipl.bin  of=$@ bs=512 count=1 conv=notrunc > /dev/null 2>&1
	@echo "load kernel..."
	@sudo mount $@ mnt
	@cat boot/boot.bin kernel.bin > os-image
	@sudo cp os-image mnt/
	@sudo umount mnt

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext ${KERNEL_OFFSET} --oformat binary $^ -o $@

kernel.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext ${KERNEL_OFFSET} $^ -o $@ 

# not working currently
debug:${IMG} kernel.elf
	${QEMU} -s -fda ${IMG} &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.asm
	$(AS) -f elf $< -o $@

%.bin: %.asm
	$(AS) -f bin $< -o $@

%.o: %.c
	$(CC) ${CFLAGS} -ffreestanding -c $< -o $@

clean:
	rm -rf os-image.sys kernel.bin kernel.elf ${IMG}
	rm -rf kernel/*.o boot/*.bin boot/*.o
	rm -rf drivers/*.o
