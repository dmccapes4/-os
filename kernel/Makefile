# kyrios kernel — bare-metal Pi 4 (BCM2711), rungs B0/B1.
CROSS   ?= aarch64-linux-gnu-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
CFLAGS  := -ffreestanding -nostdlib -nostartfiles -fno-stack-protector \
           -fno-pic -Wall -Wextra -O2 -mgeneral-regs-only
BUILD   := build

OBJS := $(BUILD)/boot.o $(BUILD)/kernel.o

all: kernel8.img

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: src/boot.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.o: src/kernel.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/kernel.elf: $(OBJS) src/link.ld
	$(CC) $(CFLAGS) -T src/link.ld $(OBJS) -o $@

kernel8.img: $(BUILD)/kernel.elf
	$(OBJCOPY) $< -O binary $@
	@echo "built $@:"
	@sha256sum $@
	@ls -l $@

clean:
	rm -rf $(BUILD) kernel8.img

.PHONY: all clean
