#include "elf.h"

#include <mm/kmem.h>

#include <common/assert.h>
#include <lib/def.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/math.h>
#include <lib/string.h>

#pragma region ELF SPEC

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
    uchar_t magic[4];
    uchar_t class;
    uchar_t data;
    uchar_t version;
    uchar_t osabi;
    uchar_t abiversion;
    uchar_t _rsv[7];
    half_t  type;
    half_t  machine;
    word_t  version1;
    addr_t  entry;
    off_t   phoff;
    off_t   shoff;
    word_t  flags;
    half_t  ehsize;
    half_t  phentsize;
    half_t  phnum;
    half_t  shentsize;
    half_t  shnum;
    half_t  shstrndx;
} 
__attribute__((packed))
elf_hdr_t;

#define ELF_MAGIC "\x7F""ELF"

typedef enum
{
    ELF_CLASS_32 = 1,
    ELF_CLASS_64 = 2
}
elf_class_t;

typedef enum
{
    ELF_DATA_ENC_LE = 1,
    ELF_DATA_ENC_BE = 2
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

// Program headers

typedef struct
{
    word_t  type;      // Segment type.
    word_t  flags;     // Segment flags.
    off_t   offset;    // Offset in file.
    addr_t  vaddr;     // Virtual address in memory.
    addr_t  paddr;     // Physical address.
    xword_t filesz;    // Size of segment in file.
    xword_t memsz;     // Size of segment in memory.
    xword_t align;     // Alignment.
}
__attribute__((packed))
ph_t;

typedef enum
{
    PH_NULL    = 0, // Unused entry
    PH_LOAD    = 1, // Loadable segment
    PH_DYNAMIC = 2, // Dynamic linking tables
    PH_INTERP  = 3, // Program interpreter path name
    PH_NOTE    = 4  // Note sections
}
ph_type_t;

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
__attribute__((packed))
sh_t;

typedef enum
{
    SH_NULL      = 0,  // Marks an unused section header.
    SH_PROGBITS  = 1,  // Contains information defined by the program.
    SH_SYMTAB    = 2,  // Contains a linker symbol table.
    SH_STRTAB    = 3,  // Contains a string table.
    SH_RELA      = 4,  // Contains “Rela” type relocation entries.
    SH_HASH      = 5,  // Contains a symbol hash table.
    SH_DYNAMIC   = 6,  // Contains dynamic linking tables.
    SH_NOTE      = 7,  // Contains note information.
    SH_NOBITS    = 8,  // Contains uninitialized space; does not occupy any space in the file.
    SH_REL       = 9,  // Contains “Rel” type relocation entries.
    SH_SHLIB     = 10, // Reserved.
    SH_DYNSYM    = 11  // Contains a dynamic loader symbol table.
}
sh_type_t;

typedef enum
{
    SH_WRITE     = 1,
    SH_ALLOC     = 2,
    SH_EXECINSTR = 4
}
sh_flag_t;

#pragma endregion

typedef struct elf_object
{
    vfs_node_t *file;
    elf_hdr_t   hdr;
}
elf_object_t;

elf_object_t *elf_read(vfs_node_t *file)
{
    ASSERT(file != NULL && file->type == VFS_NODE_FILE);
    elf_object_t *elf_obj = kmem_alloc(sizeof(elf_object_t));

    elf_obj->file = file;
    file->ops->read(file, 0, sizeof(elf_hdr_t), &elf_obj->hdr);

    return elf_obj;
}

bool elf_is_compatible(elf_object_t *elf_obj)
{
    elf_hdr_t *hdr = &elf_obj->hdr;

    if (strncmp((const char*)hdr->magic, ELF_MAGIC, 4) != 0)
    {
        log("ELF: Invalid magic number.");
        return false;
    }

    if (hdr->class != ELF_CLASS_64)
    {
        log("ELF: Only 64-bit files are supported.");
        return false;
    }

    if (hdr->data != ELF_DATA_ENC_LE)
    {
        log("ELF: Only little endian files are supported.");
        return false;
    }

    if (hdr->osabi != ELF_ABI_SYSV)
    {
        log("ELF: Only SYS-V ABI is supported for files.");
        return false;
    }

    return true;
}

void elf_load_exec(elf_object_t *elf_obj, vmm_addr_space_t *addr_space)
{
    vfs_node_t *file = elf_obj->file;
    elf_hdr_t  *hdr  = &elf_obj->hdr;

    ASSERT (hdr->type == ELF_TYPE_EXEC);

    ph_t *ph_table = kmem_alloc(hdr->phentsize * hdr->phnum);
    file->ops->read(file, hdr->phoff, hdr->phentsize * hdr->phnum, ph_table);

    for (uint i = 0; i < hdr->phnum; i++)
    {
        ph_t *ph = &ph_table[i];

        if (ph->type == PH_LOAD)
        {   
            uptr end = ph->vaddr + ph->memsz;
            uptr start = FLOOR(ph->vaddr, ARCH_PAGE_GRAN);
            u64  len = CEIL(end - start, ARCH_PAGE_GRAN);

            vmm_map_anon(addr_space, start, len, VMM_FULL);
            file->ops->read(file, ph->offset, ph->memsz, (void*)(vmm_virt_to_phys(addr_space, ph->vaddr) + HHDM));
        }
    }

    kmem_free(ph_table, sizeof(ph_t));
}

// void elf_load_reloc(elf_object_t *elf_obj, vmm_addr_space_t *as)
// {
//     vfs_node_t *file = elf_obj->file;
//     elf_hdr_t  *hdr  = &elf_obj->hdr;

//     // Allocate and read section headers.
//     sh_t *sh_table = kmem_alloc(hdr->shentsize * hdr->shnum);
//     file->ops->read(file, hdr->shoff, hdr->shentsize * hdr->shnum, sh_table);

//     // Find symbol and string tables.
//     sh_t *symtab = NULL, *strtab = NULL;
//     for (uint i = 0; i < hdr->shnum; i++)
//     {
//         sh_t *sh = &sh_table[i];

//         if (sh->type == SH_SYMTAB)
//             symtab = sh;
//         else if (sh->type == SH_STRTAB && i != hdr->shstrndx) // Ignore section names table
//             strtab = sh;
//     }

//     if (!symtab || !strtab)
//     {
//         kmem_free(sh_table);
//         panic("ELF relocatable file missing symbol or string table!");
//     }

//     // Load symbol table
//     elf_sym_t *symbols = kmem_alloc(symtab->size);
//     file->ops->read(file, symtab->offset, symtab->size, symbols);

//     // Load string table
//     char *strtab_data = kmem_alloc(strtab->size);
//     file->ops->read(file, strtab->offset, strtab->size, strtab_data);

//     // Load sections into memory
//     for (uint i = 0; i < hdr->shnum; i++)
//     {
//         sh_t *sh = &sh_table[i];

//         if (sh->flags & SH_ALLOC) // Loadable section
//         {
//             uptr start = FLOOR(sh->addr, ARCH_PAGE_GRAN);
//             u64 len = CEIL(sh->size, ARCH_PAGE_GRAN);

//             vmm_map_anon(as, start, len);
//             file->ops->read(file, sh->offset, sh->size, (void*)(vmm_virt_to_phys(as, sh->addr) + HHDM));
//         }
//     }

//     // Process relocations
//     for (uint i = 0; i < hdr->shnum; i++)
//     {
//         sh_t *sh = &sh_table[i];

//         if (sh->type == SH_REL)
//         {
//             elf_rel_t *relocs = kmem_alloc(sh->size);
//             file->ops->read(file, sh->offset, sh->size, relocs);

//             uint count = sh->size / sizeof(elf_rel_t);
//             for (uint j = 0; j < count; j++)
//             {
//                 elf_rel_t *rel = &relocs[j];
//                 u32 sym_idx = ELF_R_SYM(rel->info);
//                 u32 rel_type = ELF_R_TYPE(rel->info);

//                 elf_sym_t *sym = &symbols[sym_idx];
//                 uptr sym_value = sym->value; // Assume pre-resolved

//                 uptr *target = (uptr *)(vmm_virt_to_phys(as, rel->offset) + HHDM);

//                 switch (rel_type)
//                 {
//                     case R_X86_64_64:
//                         *target += sym_value;
//                         break;

//                     case R_X86_64_PC32:
//                         *target += sym_value - (uptr)target;
//                         break;

//                     default:
//                         panic("Unsupported relocation type!");
//                 }
//             }

//             kmem_free(relocs);
//         }
//     }

//     kmem_free(symbols);
//     kmem_free(strtab_data);
//     kmem_free(sh_table);
// }

uptr elf_get_entry(elf_object_t *elf_obj)
{
    log("entry: %#llx", elf_obj->hdr.entry);

    return (uptr)elf_obj->hdr.entry;
}
