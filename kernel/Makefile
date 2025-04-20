SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
KERNEL_ELF := $(BIN_DIR)/kernel.elf
KERNEL_SYM := $(BIN_DIR)/kernel_symbols.txt

# TOOLCHAIN

CC = clang
LD = ld.lld

# COMPILER FLAGS

C_FLAGS := 					\
	-Wall					\
	-Wextra 				\
	-ffreestanding 			\
	-fno-stack-protector	\
	-fno-stack-check		\
	-fno-lto   				\
	-fPIE					\
	-O3						\
	-g						\

LD_FLAGS := 				\
	-nostdlib				\
	-static 				\
	-gc-sections 			\
	-z max-page-size=0x1000 \
	-g 						\
	-T linker-$(ARCH).ld

AS_FLAGS := 				\
	-g

ifeq ($(ARCH), x86_64)
	C_FLAGS += 				\
		-target x86_64-elf  \
        -m64 				\
        -mgeneral-regs-only \
        -mno-red-zone

	LD_FLAGS += 			\
		-m elf_x86_64

	NASM_FLAGS =			\
		-Wall				\
		-f elf64			\
		-w-reloc-abs-dword  \
		-w-reloc-rel-dword  \
		-w-reloc-abs-qword  \
		-g
else ifeq ($(ARCH), aarch64)
	C_FLAGS +=							\
		-target aarch64-unknown-none  	\
        -mgeneral-regs-only

	LD_FLAGS += 						\
		-m aarch64elf

	AS_FLAGS += 						\
		-target aarch64-unknown-none
endif

C_FLAGS += 							\
	-I $(SRC_DIR)					\
	-std=gnu23						\
	-DPRINTF_DISABLE_SUPPORT_FLOAT  \
	-DLIMINE_API_REVISION=2

C_FILES   := $(shell cd $(SRC_DIR) && find -L * -type f -name '*.c' -not -path "arch/*/*")
AS_FILES  := $(shell cd $(SRC_DIR) && find -L * -type f -name '*.S' -not -path "arch/*/*")

ifeq ($(ARCH), x86_64)
	C_FILES   += $(shell cd $(SRC_DIR) && find -L * -type f -name '*.c' -path "arch/x86_64/*")
	AS_FILES  += $(shell cd $(SRC_DIR) && find -L * -type f -name '*.S' -path "arch/x86_64/*")
	NASM_FILES = $(shell cd $(SRC_DIR) && find -L * -type f -name '*.asm' -path "arch/x86_64/*")

	OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o) $(NASM_FILES:.asm=.asm.o))
else ifeq ($(ARCH), aarch64)
	C_FILES   += $(shell cd $(SRC_DIR) && find -L * -type f -name '*.c' -path "arch/aarch64/*")
	AS_FILES  += $(shell cd $(SRC_DIR) && find -L * -type f -name '*.S' -path "arch/aarch64/*")

	OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o))
endif

.PHONY: build
build: $(KERNEL_ELF)

.PHONY: limine
limine:
	wget -O $(SRC_DIR)/common/limine/limine.h https://raw.githubusercontent.com/limine-bootloader/limine/refs/heads/v8.x-binary/limine.h

$(KERNEL_ELF): $(OBJ_FILES)
	mkdir -p $(@D)
	$(LD) $(LD_FLAGS) -o $@ $^
	llvm-nm $@ -n -g > $(KERNEL_SYM)

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c limine
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.S.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(CC) $(AS_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	mkdir -p $(@D)
	nasm $(NASM_FLAGS) -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

distclean: clean
	rm $(SRC_DIR)/common/limine/limine.h
