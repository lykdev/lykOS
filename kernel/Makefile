KERNEL_ELF := bin/kernel.elf

# TOOLCHAIN

CC = clang

# FLAGS

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
	-I src					\
	-std=c23				\

LD_FLAGS := 				\
	-nostdlib				\
	-static 				\
	-pie					\
	--no-dynamic-linker 	\
	-z text 				\
	-z max-page-size=0x1000 \
	-T linker-$(ARCH).ld

ifeq ($(ARCH), x86_64)
	C_FLAGS += 	\
		-target x86_64-elf  \
        -m64 				\
        -march=x86-64 		\
        -mgeneral-regs-only \
		-march=x86-64		\
        -mno-80387			\
        -mno-mmx			\
        -mno-sse			\
        -mno-sse2			\
        -mno-red-zone

	LD_FLAGS += 		\
		-m elf_x86_64

	NASM_FLAGS =		\
		-Wall			\
		-f elf64		\
		-g
endif

# "USER" DEFINED FLAGS

C_FLAGS += \
	-DPRINTF_DISABLE_SUPPORT_FLOAT

# OBJECTS

C_FILES   := $(shell cd src && find -L * -type f -name '*.c' -not -path "arch/*/*")
AS_FILES  := $(shell cd src && find -L * -type f -name '*.S' -not -path "arch/*/*")

ifeq ($(ARCH), x86_64)
	C_FILES   += $(shell cd src && find -L * -type f -name '*.c' -path "arch/x86_64/*")
	AS_FILES  += $(shell cd src && find -L * -type f -name '*.S' -path "arch/x86_64/*")
	NASM_FILES = $(shell cd src && find -L * -type f -name '*.asm' -path "arch/x86_64/*")

	OBJ_FILES = $(addprefix obj/, $(C_FILES:.c=.c.o) $(AS_FILES:.S=.S.o) $(NASM_FILES:.asm=.asm.o))
endif

build: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJ_FILES)
	mkdir -p $(@D)
	ld $(LD_FLAGS) -o $@ $^

obj/%.c.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

obj/%.S.o: src/%.S
	mkdir -p $(@D)
	$(CC) $(AS_FLAGS) -o $@ $<

obj/%.asm.o: src/%.asm
	mkdir -p $(@D)
	nasm $(NASM_FLAGS) -o $@ $<

clean:
	rm -rf obj bin