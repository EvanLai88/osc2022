SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector

all: clean kernel8.img

start.o: start.S
	aarch64-elf-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-elf-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-elf-ld -nostdlib -nostartfiles start.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-elf-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.* *.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

debug:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -S -s
	gdb-multiarch