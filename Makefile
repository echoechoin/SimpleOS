AS   := nasm
DASM := ndisasm
CC   := i386-elf-gcc
LD   := i386-elf-ld
GDB  := i386-elf-gdb

CFLAGS := -g
KERNEL_OFFSET := 0x8200 # `指示`内核起始地址
IMG := os-image.bin

C_SOURCES := $(wildcard kernel/*.c drivers/*.c)
HEADERS := $(wildcard kernel/*.h drivers/*.h)
OBJ = ${C_SOURCES:.c=.o}

all: ${IMG}

${IMG}: boot/boot.bin kernel.bin
	cat $^ > $@

kernel.bin: boot/kernel_entry.o ${OBJ}
	$(LD) -Ttext ${KERNEL_OFFSET} --oformat binary -o $@ $^

# 用于调试内核
kernel.elf: boot/kernel_entry.o ${OBJ}
	$(LD) -o $@ -Ttext ${KERNEL_OFFSET} $^

debug: ${IMG} kernel.elf
	qemu-system-i386 -s -fda ${IMG} &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.c ${HEADERS}
	$(CC) ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	$(AS) -f elf -o $@ $<

%.bin: %.asm
	$(AS) -f bin  -o $@ $<

clean:
	rm -rf kernel.bin kernel.elf ${IMG}
	rm -rf kernel/*.o boot/*.bin boot/*.o
	rm -rf drivers/*.o
