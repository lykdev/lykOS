KERNEL_ELF := bin/kernel.elf

# TOOLCHAIN

CC = clang

# COMPILER FLAGS

C_FLAGS := 					\
	-Wall					\
	-Wextra 				\
	-ffreestanding 			\
	-fno-stack-protector	\
	-fno-stack-check		\
	-fno-lto				\
	-fPIE					\
	-O2						\
	-g						\

LD_FLAGS := 				\
	-nostdlib				\
	-static 				\
	-gc-sections 			\
	-z max-page-size=0x1000 \
	-T linker-$(ARCH).ld

# ARCH-SPECIFIC COMPILER FLAGS

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

# CODE-SPECIFIC COMPILER FLAGS

C_FLAGS += 							\
	-I src							\
	-std=gnu23						\
	-DPRINTF_DISABLE_SUPPORT_FLOAT

# OBJECTS

C_FILES   := $(shell cd src && find -L * -type f -name '*.c' -not -path "arch/*/*")
AS_FILES  := $(shell cd src && find -L * -type f -name '*.S' -not -path "arch/*/*")

ifeq ($(ARCH), x86_64)
	C_FILES   += $(shell cd src && find -L * -type f -name '*.c' -path "arch/x86_64/*")
	AS_FILES  += $(shell cd src && find -L * -type f -name '*.S' -path "arch/x86_64/*")
	NASM_FILES = $(shell cd src && find -L * -type f -name '*.asm' -path "arch/x86_64/*")

	OBJ_FILES = $(addprefix obj/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o) $(NASM_FILES:.asm=.asm.o))
else ifeq ($(ARCH), aarch64)
	C_FILES   += $(shell cd src && find -L * -type f -name '*.c' -path "arch/aarch64/*")
	AS_FILES  += $(shell cd src && find -L * -type f -name '*.S' -path "arch/aarch64/*")

	OBJ_FILES = $(addprefix obj/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o))
endif

# BUILDING

.PHONY: build
build: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJ_FILES)
	mkdir -p $(@D)
	ld.lld $(LD_FLAGS) -o $@ $^

obj/%.c.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

obj/%.S.o: src/%.S
	mkdir -p $(@D)
	$(CC) $(AS_FLAGS) -c -o $@ $<

obj/%.asm.o: src/%.asm
	mkdir -p $(@D)
	nasm $(NASM_FLAGS) -o $@ $<

clean:
	rm -rf obj bin
