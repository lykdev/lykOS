CURRENT_DIR := $(shell basename $(PWD))
ELF_FILE := bin/$(CURRENT_DIR).elf

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
	-fno-lto				\
	-fPIC					\
	-O2						\
	-g						\

LD_FLAGS := 				\
	-nostdlib				\
	-static 				\
	-gc-sections 			\
	-z max-page-size=0x1000 \
	-r

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
	-std=gnu23

# OBJECTS

C_FILES = $(shell cd src && find * -type f -name "*.c")
OBJ_FILES = $(addprefix obj/, $(C_FILES:.c=.o))

# BUILDING

.PHONY: build
build: $(ELF_FILE)

$(ELF_FILE): $(OBJ_FILES)
	mkdir -p $(@D)
	$(LD) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c -o $@ $<

clean:
	rm -rf obj/ bin/
