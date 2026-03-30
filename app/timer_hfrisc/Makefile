CROSS_COMPILER = riscv64-unknown-elf-
CC = $(CROSS_COMPILER)gcc
AS = $(CROSS_COMPILER)as
LD = $(CROSS_COMPILER)ld
SIZE = $(CROSS_COMPILER)size
OBJCOPY = $(CROSS_COMPILER)objcopy
OBJDUMP = $(CROSS_COMPILER)objdump
GDB = $(CROSS_COMPILER)gdb

CFLAGS = -march=rv32e -mabi=ilp32e -ffixed-tp -ffixed-a5 -mstrict-align \
	-fno-builtin -fno-stack-protector -DCPU_SPEED=25000000 \
	-nostdlib -nostartfiles -ffreestanding \
	-fdata-sections -ffunction-sections -Os \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-qual \
	-Wimplicit-fallthrough \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wno-unused-parameter 
ASFLAGS = -march=rv32e -mabi=ilp32e
LDFLAGS = -T$(BSP_DIR)/hf-risc.ld -melf32lriscv $(LDFLAGS_STRIP) \
	-nostdlib -static --gc-sections 
INCDIRS = -I$(ARCH_DIR) -I$(BSP_DIR) -I$(LIBC_DIR)/../inc \
	-I$(DRIVERS_DIR)/hal/uart -I$(DRIVERS_DIR)/native/uart_hfrisc \
	-I$(APP_DIR)
LIBCFLAGS = -DMALLOC_FULL -DSTDIO_EXTRA -DSTDLIB_EXTRA -DSTRING_EXTRA
CFGFLAGS = 

BASE_DIR = ../..
ARCH_DIR = $(BASE_DIR)/arch/riscv/hf-risc
DRIVERS_DIR = $(BASE_DIR)/drivers
BSP_DIR = $(BASE_DIR)/bsp/hf-risc
LIBC_DIR = $(BASE_DIR)/lib/libc/src
APP_DIR = .
BUILD_DIR = .

ARCH_OBJS = $(ARCH_DIR)/start.o $(ARCH_DIR)/trap.o \
	$(ARCH_DIR)/muldiv.o $(ARCH_DIR)/interrupt.o
BSP_OBJS = $(BSP_DIR)/hf-risc.o
DRIVERS_OBJS = $(DRIVERS_DIR)/hal/uart/uart_hal.o \
	$(DRIVERS_DIR)/native/uart_hfrisc/uart_hfrisc.o
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

.PHONY: all clean
