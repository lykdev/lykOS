BUILD_DIR ?= build
INSTALL_DIR ?= install
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_SYM := $(BUILD_DIR)/kernel_symbols.txt

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
	-O2						\
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
	-I src					\
	-std=gnu23						\
	-DPRINTF_DISABLE_SUPPORT_FLOAT  \
	-DLIMINE_API_REVISION=2

C_FILES   := $(shell cd src && find -L * -type f -name '*.c' -not -path "arch/*/*")
AS_FILES  := $(shell cd src && find -L * -type f -name '*.S' -not -path "arch/*/*")

ifeq ($(ARCH), x86_64)
	C_FILES   += $(shell cd src && find -L * -type f -name '*.c' -path "arch/x86_64/*")
	AS_FILES  += $(shell cd src && find -L * -type f -name '*.S' -path "arch/x86_64/*")
	NASM_FILES = $(shell cd src && find -L * -type f -name '*.asm' -path "arch/x86_64/*")

	OBJ_FILES = $(addprefix $(BUILD_DIR)/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o) $(NASM_FILES:.asm=.asm.o))
else ifeq ($(ARCH), aarch64)
	C_FILES   += $(shell cd src && find -L * -type f -name '*.c' -path "arch/aarch64/*")
	AS_FILES  += $(shell cd src && find -L * -type f -name '*.S' -path "arch/aarch64/*")

	OBJ_FILES = $(addprefix $(BUILD_DIR)/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o))
endif

.PHONY: build
build: $(OBJ_FILES)
	$(LD) $(LD_FLAGS) -o $(KERNEL_ELF) $^
	llvm-nm $(KERNEL_ELF) -n -g > $(KERNEL_SYM)

.PHONY: limine
limine: src/common/limine/limine.h

src/common/limine/limine.h:
	wget -O src/common/limine/limine.h https://raw.githubusercontent.com/KM198912/limine/refs/heads/v8.x-binary/limine.h

$(BUILD_DIR)/%.c.o: src/%.c src/common/limine/limine.h
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: src/%.S
	mkdir -p $(@D)
	$(CC) $(AS_FLAGS) -c -o $@ $<

$(BUILD_DIR)/%.asm.o: src/%.asm
	mkdir -p $(@D)
	nasm $(NASM_FLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

distclean: clean
	rm src/common/limine/limine.h

.PHONY: install
install:
	install -d $(INSTALL_DIR)
	install $(KERNEL_ELF) $(INSTALL_DIR)
	install $(KERNEL_SYM) $(INSTALL_DIR)
