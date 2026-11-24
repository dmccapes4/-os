# kyrios v0.0.0 (development; v0.0.1 is the boot milestone, declared by the operator)
#
#   make          the arm64 image for lirvol (Raspberry Pi 5)
#   make check    build the system code for the bench and test it
#   make clean
#
# Output goes to _build/, which the repository ignores.

VERSION := 0.0.0
CROSS   ?= aarch64-linux-gnu-
B       := _build
PYTHON  ?= ../.venv/bin/python
IMAGE   := $(B)/kyrios-v$(VERSION).img

KERNEL_C := kernel/lib.c kernel/sha256.c kernel/walk.c kernel/bcm2712.c
KERNEL_S := kernel/start.S kernel/vectors.S
SYSTEM_C := system/store.c system/record.c system/decay.c system/interact.c system/main.c
HEADERS  := $(wildcard kernel/*.h system/*.h host/*.h)

# -mgeneral-regs-only: no floating point or vector registers anywhere.
# -mstrict-align: with the MMU off, memory is treated as device memory and
#  an unaligned access faults.
CFLAGS := -ffreestanding -nostdlib -nostartfiles -fno-stack-protector -fno-pic \
          -fno-builtin -fno-tree-loop-distribute-patterns -mgeneral-regs-only -mstrict-align -O2 -Wall -Wextra -std=gnu11

OBJS := $(patsubst %.c,$(B)/arm64/%.o,$(KERNEL_C) $(SYSTEM_C)) \
        $(patsubst %.S,$(B)/arm64/%.o,$(KERNEL_S))

HOST_C := kernel/lib.c kernel/sha256.c kernel/walk.c $(SYSTEM_C) host/host_board.c host/test_kyrios.c
HOST_CFLAGS := -DKYRIOS_HOST -O1 -g -Wall -Wextra -std=gnu11 -fsanitize=address,undefined -fno-omit-frame-pointer

all: $(IMAGE)

$(B)/arm64/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

$(B)/arm64/%.o: %.S
	@mkdir -p $(dir $@)
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

$(B)/kyrios.elf: $(OBJS) kernel/link.ld
	$(CROSS)ld -nostdlib -T kernel/link.ld -Map $(B)/kyrios.map -o $@ $(OBJS)

$(IMAGE): $(B)/kyrios.elf
	$(CROSS)objcopy -O binary $< $@
	@sha256sum $@
	@ls -l $@

$(B)/host/test_kyrios: $(HOST_C) $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(HOST_CFLAGS) $(HOST_C) -o $@

check: $(B)/host/test_kyrios
	$(B)/host/test_kyrios $(B)/host
	$(PYTHON) host/check_memory.py $(B)/host

clean:
	rm -rf $(B)

.PHONY: all check clean
