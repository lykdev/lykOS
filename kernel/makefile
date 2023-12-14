# Directories

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Executable name

KERNEL = $(BIN_DIR)/kernel.elf

# Toolchain

ifeq ($(ARCH), x86_64)
	CC = x86_64-elf-gcc
	AS = x86_64-elf-as
	NASM = nasm
	LD = x86_64-elf-ld
else ifeq ($(ARCH), aarch64)
	CC = aarch64-elf-gcc
	AS = aarch64-elf-as
	LD = aarch64-elf-ld
endif

# Compiler flags

C_FLAGS = \
	-Wall \
	-Wextra \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fPIE \
	-g \
	-I ./src

LD_FLAGS = \
	-nostdlib \
	-static \
	-pie \
	--no-dynamic-linker \
	-z text \
	-z max-page-size=0x1000 \
	-T linker-$(ARCH).ld

ifeq ($(ARCH), x86_64)
	C_FLAGS += \
		-m64 \
		-march=x86-64 \
		-mno-80387 \
		-mno-mmx \
		-mno-sse \
		-mno-sse2 \
		-mno-red-zone		

	NASM_FLAGS = \
		-Wall \
		-f elf64

	LD_FLAGS +=	\
		-m elf_x86_64
else ifeq ($(ARCH), aarch64)
	C_FLAGS += \
		-mgeneral-regs-only

	LD_FLAGS +=	\
		-m aarch64elf
endif

# Source files
C_FILES = $(shell cd src && find -L * -type f -name '*.c' -not -path "arch/*")
AS_FILES = $(shell cd src && find -L * -type f -name '*.S' -not -path "arch/*")

ifeq ($(ARCH), x86_64)
	C_FILES += $(shell cd src && find -L * -type f -name '*.c' -path "*arch/x86_64/*")
	AS_FILES += $(shell cd src && find -L * -type f -name '*.S' -path "arch/x86_64/*")
	NASM_FILES = $(shell cd src && find -L * -type f -name '*.asm')

	OBJ_FILES = $(addprefix obj/,$(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o) $(NASM_FILES:.asm=.asm.o))
else ifeq ($(ARCH), aarch64)
	C_FILES += $(shell cd src && find -L * -type f -name '*.c' -path "arch/aarch64/*")
	AS_FILES += $(shell cd src && find -L * -type f -name '*.S' -path "arch/aarch64/*")

	OBJ_FILES = $(addprefix obj/,$(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o))
endif

# Compilation

all: debug $(KERNEL)

debug:
	@echo "Building kernel for $(ARCH)."

$(KERNEL): $(OBJ_FILES)
	mkdir -p $(@D)
	$(LD) $(LD_FLAGS) -o $@ $^

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.S.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(AS) $(AS_FLAGS) -o $@ $<

$(OBJ_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	mkdir -p $(@D)
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Clean

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) 