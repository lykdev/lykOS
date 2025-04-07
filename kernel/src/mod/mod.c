#include "mod.h"

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/def.h>
#include <lib/string.h>
#include <mm/kmem.h>
#include <mm/pmm.h>
#include <mod/elf.h>

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

module_t *module_load(vfs_node_t *file)
{
    module_t *module = kmem_alloc(sizeof(module_t));

    Elf64_Ehdr ehdr;
    file->ops->read(file, 0, sizeof(Elf64_Ehdr), &ehdr);

    if (!elf_check_compatibility(&ehdr))
    {
        log("Incompatible ELF file!");
        return NULL;
    }
    if (ehdr.e_type != ET_REL)
    {
        log("Provided ELF file isn't a relocatable object file!");
        return NULL;
    }

    Elf64_Shdr *shdr = kmem_alloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
    
    // Allocate memory for the program sections.
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
            //void *mem = kmem_alloc(section->sh_size);
            void *mem = (void*)((uptr)pmm_alloc(1) + HHDM);
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
        return NULL;
    }
    void *symtab = kmem_alloc(symtab_hdr->sh_size);
    file->ops->read(file, symtab_hdr->sh_offset, symtab_hdr->sh_size, symtab);

    // String table.
    Elf64_Shdr *strtab_hdr = &shdr[symtab_hdr->sh_link];
    if (!strtab_hdr)
    {
        log("Missing string table!");
        return NULL;
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
            sym->st_value = mod_resolve_sym(name);
            log("resolved to: %llx", sym->st_value);
            break;
        case SHN_ABS:
            break;
        case SHN_COMMON:
            log("Warning: unexpected common symbol.");
            break;
        default:
            sym->st_value += section_addr[sym->st_shndx];

            if (strcmp(name, "__module_install") == 0)
                module->install = (void(*)())sym->st_value;
            else if (strcmp(name, "__module_destroy") == 0)
                module->destroy = (void(*)())sym->st_value;
            else if (strcmp(name, "__module_probe") == 0)
                module->probe   = (void(*)())sym->st_value;

            break;
        }
    }

    // Load relocation sections.
    for (size_t i = 0; i < ehdr.e_shnum; i++)
    {
        Elf64_Shdr *section = &shdr[i];

        if (section->sh_type != SHT_RELA)
            continue;

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
                    return NULL;
            }

            memcpy(addr, &value, reloc_size);
        }
    }

    log("Relocatable ELF loaded successfully.");
    return module;
}