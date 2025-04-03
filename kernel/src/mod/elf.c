#include "elf.h"

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <mm/kmem.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/def.h>
#include <lib/math.h>
#include <lib/string.h>

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;
typedef i32 Elf64_Sword;
typedef u64 Elf64_Xword;
typedef i64 Elf64_Sxword;
typedef u8  Elf64_Byte;

typedef struct
{
    unsigned char e_ident[16];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
}
__attribute__((packed))
Elf64_Ehdr;

#define EI_MAG0        0
#define EI_MAG1        1
#define EI_MAG2        2
#define EI_MAG3        3
#define EI_CLASS       4
#define EI_DATA        5
#define EI_VERSION     6
#define EI_OSABI       7
#define EI_ABIVERSION  8
#define EI_PAD         9
#define EI_NIDENT     16

#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define EV_CURRENT 1

#define ELFOSABI_SYSV       0
#define ELFOSABI_STANDALONE 255

#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3

#define EM_x86_64  0x3E
#define EM_AARCH64 0xB7

// Sections

typedef struct
{
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
}
__attribute__((packed))
Elf64_Shdr;

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11

#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4

#define SHN_UNDEF  0x0
#define SHN_ABS    0xFFF1
#define SHN_COMMON 0xFFF2

// Symbol Table

typedef struct
{
    Elf64_Word st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half st_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
}
__attribute__((packed))
Elf64_Sym;

#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3

// Relocations

typedef struct
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
}
__attribute__((packed))
Elf64_Rel;

typedef struct
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
}
__attribute__((packed))
Elf64_Rela;

#define ELF64_R_SYM(I)     ((I) >> 32)
#define ELF64_R_TYPE(I)    ((I) & 0xFFFFFFFFl)
#define ELF64_R_INFO(S, T) (((S) << 32) + ((T) & 0xFFFFFFFFl))

#define R_X86_64_NONE   0
#define R_X86_64_64     1
#define R_X86_64_PC32   2
#define R_X86_64_PLT32  4
#define R_X86_64_32    10
#define R_X86_64_32S   11
#define R_X86_64_PC64  24

// Code

bool elf_check_compatibility(Elf64_Ehdr *hdr)
{
    // Check if it's and ELF file first.
    if (memcmp(hdr->e_ident, "\x7F""ELF", 4))
    {
        log("Invalid ELF magic number.");
        return false;
    }

    bool ret = true;
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        log("Unsupported class.");
        ret = false;
    }
    if(hdr->e_ident[EI_DATA] != ELFDATA2LSB)
    {
		log("Unsupported endianness.");
		ret = false;
	}
#if defined(__x86_64__)
    if (hdr->e_machine != EM_x86_64)
#elif defined(__aarch64__)
    if (hdr->e_machine != EM_AARCH64)
#endif
    {
        log("Unsupported target architecture.");
        ret = false;
    }
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT)
    {
        log("Unsupported object file format version.");
        ret = false;
    }

    return ret;
}

bool elf_load_relocatable(vfs_node_t *file)
{
    Elf64_Ehdr ehdr;
    file->ops->read(file, 0, sizeof(Elf64_Ehdr), &ehdr);

    if (!elf_check_compatibility(&ehdr))
    {
        log("Incompatible ELF file!");
        return false;
    }
    if (ehdr.e_type != ET_REL)
    {
        log("Provided ELF file isn't a relocatable object file!");
        return false;
    }

    Elf64_Shdr *shdr = kmem_alloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
    
    uptr section_addr[ehdr.e_shnum];

    for(int i = 0; i < ehdr.e_shnum; i++)
    {
        file->ops->read(file, ehdr.e_shoff + (ehdr.e_shentsize * i), sizeof(Elf64_Shdr), &shdr[i]);
        Elf64_Shdr *section = &shdr[i];

        if (section->sh_type == SHT_NOBITS
        &&  section->sh_size != 0
        &&  section->sh_flags & SHF_ALLOC)
        {
            void *mem = kmem_alloc(section->sh_size);
            ASSERT(mem);
            memset(mem, 0, section->sh_size);
            section_addr[i] = (uptr)mem;
            log("Loaded SHT_NOBITS section %d to memory at %p", i, mem);
        }
        if (section->sh_type == SHT_PROGBITS
        &&  section->sh_size != 0
        &&  section->sh_flags & SHF_ALLOC)
        {
            void *mem = kmem_alloc(section->sh_size);
            ASSERT(mem);
            file->ops->read(file, section->sh_offset, section->sh_size, mem);
            section_addr[i] = (uptr)mem;
            log("Loaded SHT_PROGBITS section %d to memory at %p", i, mem);
        }
    }

    // Symbol table.
    Elf64_Shdr *symtab_hdr = NULL;
    for (int i = 0; i < ehdr.e_shnum; i++)
        if (shdr[i].sh_type == SHT_SYMTAB)
            symtab_hdr = &shdr[i];                 
    if (!symtab_hdr)
    {
        log("Missing symbol table!");
        return false;
    }
    void *symtab = kmem_alloc(symtab_hdr->sh_size);
    file->ops->read(file, symtab_hdr->sh_offset, symtab_hdr->sh_size, symtab);

    // String table.
    Elf64_Shdr *strtab_hdr = &shdr[symtab_hdr->sh_link];
    if (!strtab_hdr)
    {
        log("Missing string table!");
        return false;
    }
    char *strtab = kmem_alloc(strtab_hdr->sh_size);
    file->ops->read(file, strtab_hdr->sh_offset, strtab_hdr->sh_size, strtab);

    // Resolve symbols.
    size_t sym_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
    for (size_t i = 0; i < sym_count; i++)
    {
        Elf64_Sym  *sym  = (Elf64_Sym*)(symtab + (i * symtab_hdr->sh_entsize));
        const char *name = &strtab[sym->st_name];

        log("sym: %s", name);

        switch (sym->st_shndx)
        {
        case SHN_UNDEF:
            break;
        case SHN_ABS:
            break;
        case SHN_COMMON:
            log("Warning: unexpected common symbol.");
            break;
        default:
            sym->st_value += section_addr[sym->st_shndx];
            break;
        }
    }

    // Load relocation sections.
    for (size_t i = 0; i < ehdr.e_shnum; i++)
    {
        Elf64_Shdr *section = &shdr[i];

        if (section->sh_type == SHT_RELA)
        {
            Elf64_Rela rela_entries[section->sh_size / section->sh_entsize];
            file->ops->read(file, section->sh_offset, section->sh_size, rela_entries);

            for (uint j = 0; j < section->sh_size / section->sh_entsize; j++)
            {
                Elf64_Rela *rela = &rela_entries[j];
                Elf64_Sym  *sym  = &symtab[ELF64_R_SYM(rela->r_info)];
                
                void *addr = (void*)(section_addr[section->sh_info] + rela->r_offset);
                uptr value = sym->st_value + rela->r_addend;
                u64  reloc_size;

                switch (ELF64_R_TYPE(rela->r_info))
                {
                    case R_X86_64_64:
                        reloc_size = 8;
                        break;
                    case R_X86_64_PC32:
                    case R_X86_64_PLT32:
                        value -= (uptr)addr;
                        reloc_size = 4;
                        break;
                    case R_X86_64_32:
                    case R_X86_64_32S:
                        reloc_size = 4;
                        break;
                    case R_X86_64_PC64:
                        value -= (uptr)addr;
                        reloc_size = 8;
                        break;                    
                    default:
                        log("Unsupported relocation type: 0x%x.", ELF64_R_TYPE(rela->r_info));
                        return false;
                }

                memcpy(addr, &value, reloc_size);
            }
        }
    }

    

    log("Relocatable ELF loaded successfully.");
    return true;
}