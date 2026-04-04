CROSS_COMPILER = riscv64-unknown-elf-
CC = $(CROSS_COMPILER)gcc
AS = $(CROSS_COMPILER)as
LD = $(CROSS_COMPILER)ld
SIZE = $(CROSS_COMPILER)size
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump
GDB = $(CROSS_COMPILER)gdb

CFLAGS = -march=rv64ima_zicsr -mabi=lp64 -mcmodel=medany \
	-fno-builtin -fno-stack-protector -DCPU_SPEED=25000000 \
	-nostdlib -nostartfiles -ffreestanding \
	-fdata-sections -ffunction-sections -O0 -g \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wimplicit-fallthrough \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wno-unused-parameter 
ASFLAGS = -march=rv64ima_zicsr -mabi=lp64
LDFLAGS = -T$(BSP_DIR)/riscv64.ld $(LDFLAGS_STRIP) \
	-nostdlib -static --gc-sections 
INCDIRS = -I$(ARCH_DIR) -I$(BSP_DIR) -I$(LIBC_DIR)/../inc \
	-I$(APP_DIR)
LIBCFLAGS = -DMALLOC_FULL -DSTDIO_EXTRA -DSTDLIB_EXTRA -DSTRING_EXTRA
CFGFLAGS = 

BASE_DIR = ../..
ARCH_DIR = $(BASE_DIR)/arch/riscv/riscv64
DRIVERS_DIR = $(BASE_DIR)/drivers
BSP_DIR = $(BASE_DIR)/bsp/qemu-virt
LIBC_DIR = $(BASE_DIR)/lib/libc/src
APP_DIR = .
BUILD_DIR = .

ARCH_OBJS = $(ARCH_DIR)/start.o $(ARCH_DIR)/trap.o \
	$(ARCH_DIR)/interrupt.o
BSP_OBJS = $(BSP_DIR)/qemu-virt.o
DRIVERS_OBJS =
LIBC_OBJS = $(LIBC_DIR)/stdio.o $(LIBC_DIR)/stdlib.o $(LIBC_DIR)/string.o
KERNEL_OBJS = 
APP_OBJS = $(APP_DIR)/main.o

ALL_OBJS = $(ARCH_OBJS) $(DRIVERS_OBJS) $(BSP_OBJS) $(LIBC_OBJS) \
	$(KERNEL_OBJS) $(APP_OBJS)

all: main.elf main.bin main.dump main.size

main.elf: $(ALL_OBJS)
	$(LD) $(LDFLAGS) -N -o $(BUILD_DIR)/$@ $(ALL_OBJS)

main.bin: main.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/$< $(BUILD_DIR)/$@

main.dump: main.elf
	$(OBJDUMP) -d -s $(BUILD_DIR)/$< > $(BUILD_DIR)/$@
	
main.size: main.elf
	$(SIZE) $(BUILD_DIR)/main.elf

%.o: %.c
	$(CC) $(CFLAGS) $(INCDIRS) $(LIBCFLAGS) $(CFGFLAGS) -c -o $@ $<

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(ALL_OBJS) $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.dump
	
run: main.elf
	qemu-system-riscv64 -machine virt -nographic -bios none \
	-kernel $(BUILD_DIR)/main.elf
	
debug: main.elf
	qemu-system-riscv64 -machine virt -nographic -bios none \
	-kernel $(BUILD_DIR)/main.elf -S -s

debug_gdb: main.elf
	$(GDB) $(BUILD_DIR)/main.elf -ex "target remote : 1234"

.PHONY: all clean
