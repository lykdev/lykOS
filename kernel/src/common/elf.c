#include "elf.h"

#include <core/tasking/proc.h>

#include <utils/log.h>

typedef u64 addr_t;
typedef u64 off_t;
typedef u16 half_t;
typedef u32 word_t;
typedef i32 sword_t;
typedef u64 xword_t;
typedef i64 sxword_t;
typedef u8  uchar_t;

typedef struct
{
    uchar_t magic[4];    // Magic number.
    uchar_t class;       // Class.
    uchar_t data;        // Data encoding.
    uchar_t version;     // Object file version.
    uchar_t osabi;       // OS/ABI.
    uchar_t abiversion;  // ABI version.
    uchar_t _rsv[7];     // Padding.
    half_t  type;        // Object file type.
    half_t  machine;     // Machine type.
    word_t  version1;    // Object file version.
    addr_t  entry;       // Entry point address.
    off_t   phoff;       // Program header offset.
    off_t   shoff;       // Section header offset.
    word_t  flags;       // Processor-specific flags.
    half_t  ehsize;      // ELF header size.
    half_t  phentsize;   // Size of program header entry.
    half_t  phnum;       // Number of program header entries.
    half_t  shentsize;   // Size of section header entry.
    half_t  shnum;       // Number of section header entries.
    half_t  shstrndx;    // Section name string table index.
} __attribute__((packed))
elf_hdr_t;

typedef enum
{
    ELF_CLASS_32 = 1,
    ELF_CLASS_64 = 2
}
elf_class_t;

typedef enum
{
    ELF_DATA_ENC_LE = 1, // Object file data structures are little-endian.
    ELF_DATA_ENC_BE = 2  // Object file data structures are big-endian.
}
elf_data_enc_t;

typedef enum
{
    ELF_ABI_SYSV = 0,  // System V ABI.
}
elf_abi_t;

typedef enum
{
    ELF_TYPE_NONE = 0, // No file type.
    ELF_TYPE_REL  = 1, // Relocatable object file.
    ELF_TYPE_EXEC = 2, // Executable file.
    ELF_TYPE_DYN  = 3, // Shared object file.
    ELF_TYPE_CORE = 4  // Core file.
}
elf_type_t;

// Sections

typedef struct
{
    word_t  name;      // Section name.
    word_t  type;      // Section type.
    xword_t flags;     // Section attributes.
    addr_t  addr;      // Virtual address in memory.
    off_t   offset;    // Offset in file.
    xword_t size;      // Size of section.
    word_t  link;      // Link to other section.
    word_t  info;      // Miscellaneous information.
    xword_t addralign; // Address alignment boundary
    xword_t entsize;   // Size of entries, if section has table.
}
section_t;

typedef enum
{
    SECTION_TYPE_NULL      = 0,  // Marks an unused section header.
    SECTION_TYPE_PROGBITS  = 1,  // Contains information defined by the program.
    SECTION_TYPE_SYMTAB    = 2,  // Contains a linker symbol table.
    SECTION_TYPE_STRTAB    = 3,  // Contains a string table.
    SECTION_TYPE_RELA      = 4,  // Contains “Rela” type relocation entries.
    SECTION_TYPE_HASH      = 5,  // Contains a symbol hash table.
    SECTION_TYPE_DYNAMIC   = 6,  // Contains dynamic linking tables.
    SECTION_TYPE_NOTE      = 7,  // Contains note information.
    SECTION_TYPE_NOBITS    = 8,  // Contains uninitialized space; does not occupy any space in the file.
    SECTION_TYPE_REL       = 9,  // Contains “Rel” type relocation entries.
    SECTION_TYPE_SHLIB     = 10, // Reserved.
    SECTION_TYPE_DYNSYM    = 11  // Contains a dynamic loader symbol table.
}
section_type_t;

bool elf_is_compatible(elf_hdr_t *hdr)
{
    

    return true;
}

bool elf_load(vfs_node_t *file, vmm_addr_space_t *addr_space)
{
    if (file->type != VFS_NODE_FILE)
        return false;

    elf_hdr_t hdr;
    file->ops->read(file, 0, sizeof(elf_hdr_t), &hdr);

    log("%c%c%c%c", hdr.magic[0], hdr.magic[1], hdr.magic[2], hdr.magic[3]);

    return true;
}
