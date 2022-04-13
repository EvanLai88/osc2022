CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -I include

SRC_DIR = src
ROOTFS_DIR = rootfs
OUT_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)

all: clean rootfs.img kernel8.img

$(OUT_DIR)/%.o: $(SRC_DIR)/%.S
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ROOTFS_DIR)/%.o: $(ROOTFS_DIR)/%.S
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/boot.o: bootloader/boot.S
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/bootloader.o: bootloader/bootloader.c
	$(CC) $(CFLAGS) -c $< -o $@

#bootloader.img: $(OUT_DIR)/boot.o $(OUT_DIR)/bootloader.o $(OUT_DIR)/lib.o $(OUT_DIR)/uart.o
#	$(LD) -nostdlib -nostartfiles $^ -T bootloader/link.ld -o bootloader.elf
#	$(OBJCOPY) -O binary bootloader.elf bootloader.img

kernel8.img: $(OUT_DIR)/start.o $(OUT_DIR)/exceptionVectorTable.o $(OBJS)
	$(LD) -nostdlib -nostartfiles $^ -T link.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

rootfs.img: $(ROOTFS_DIR)/user.o
	$(LD) -nostdlib -nostartfiles $^ -T $(ROOTFS_DIR)/link.ld -o $(ROOTFS_DIR)/user.elf
	$(OBJCOPY) -O binary $(ROOTFS_DIR)/user.elf $(ROOTFS_DIR)/user.img
	./makeCPIO.sh

clean:
#	rm bootloader.* >/dev/null 2>/dev/null || true
	rm kernel8.* >/dev/null 2>/dev/null || true
	rm obj/*.o >/dev/null 2>/dev/null || true
	rm user.* >/dev/null 2>/dev/null || true
	rm *.cpio >/dev/null 2>/dev/null || true

run_kernel:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

run:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel bootloader.img -display none -serial null -serial stdio

debug_kernel:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -S -s

debug:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel bootloader.img -display none -serial null -serial stdio -S -s

run_pty:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel bootloader.img -display none -serial null -serial pty

debug_pty:
	qemu-system-aarch64 -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -M raspi3 -kernel bootloader.img -display none -serial null -serial pty -S -s