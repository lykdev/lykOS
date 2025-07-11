#include "module.h"

#include <common/elf.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <lib/def.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <sys/ksym.h>

list_t g_mod_module_list = LIST_INIT;

module_t *module_load(vfs_node_t *file)
{
    log("Loading module `%s`.", file->name);
    module_t module;

    Elf64_Ehdr ehdr;
    if (file->file_ops->read(file, 0, &ehdr, sizeof(Elf64_Ehdr)) != sizeof(Elf64_Ehdr))
    {
        log("Could not read file header!");
        return NULL;
    };

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

    // Allocate memory for the sections and save the address.
    CLEANUP Elf64_Shdr *shdr = heap_alloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
    CLEANUP uptr *section_addr = heap_alloc(ehdr.e_shnum * sizeof(uptr));

    for(int i = 0; i < ehdr.e_shnum; i++)
    {
        file->file_ops->read(file, ehdr.e_shoff + (ehdr.e_shentsize * i), &shdr[i], sizeof(Elf64_Shdr));
        Elf64_Shdr *section = &shdr[i];

        if (section->sh_type == SHT_PROGBITS
        &&  section->sh_size != 0
        &&  section->sh_flags & SHF_ALLOC)
        {
            u64 size = CEIL(section->sh_size, ARCH_PAGE_GRAN);
            void *mem = vmm_map_vnode(
                g_vmm_kernel_addr_space,
                0,
                size,
                VMM_PROT_FULL,
                VMM_MAP_ANON | VMM_MAP_POPULATE | VMM_MAP_PRIVATE,
                NULL,
                0
            );
            file->file_ops->read(file, section->sh_offset, mem, section->sh_size);

            section_addr[i] = (uptr)mem;
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
    CLEANUP void *symtab = heap_alloc(symtab_hdr->sh_size);
    file->file_ops->read(file, symtab_hdr->sh_offset, symtab, symtab_hdr->sh_size);

    // String table.
    Elf64_Shdr *strtab_hdr = &shdr[symtab_hdr->sh_link];
    if (!strtab_hdr)
    {
        log("Missing string table!");
        return NULL;
    }
    CLEANUP char *strtab = heap_alloc(strtab_hdr->sh_size);
    file->file_ops->read(file, strtab_hdr->sh_offset, strtab, strtab_hdr->sh_size);

    // Resolve symbols.
    size_t sym_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
    for (size_t i = 1; i < sym_count; i++)
    {
        Elf64_Sym  *sym  = (Elf64_Sym*)(symtab + (i * symtab_hdr->sh_entsize));
        const char *name = &strtab[sym->st_name];

        switch (sym->st_shndx)
        {
        case SHN_UNDEF:
            sym->st_value = ksym_resolve_symbol(name);
            if (sym->st_value == 0)
            {
                log("Symbol `%s` could not be resolved!", name);
                return NULL;
            }
            break;
        case SHN_ABS:
            break;
        case SHN_COMMON:
            log("Warning: unexpected common symbol.");
            break;
        default:
            sym->st_value += section_addr[sym->st_shndx];

            if (strcmp(name, "__module_install") == 0)
                module.install = (void(*)())sym->st_value;
            else if (strcmp(name, "__module_destroy") == 0)
                module.destroy = (void(*)())sym->st_value;
            else if (strcmp(name, "__module_probe") == 0)
                module.probe   = (bool(*)())sym->st_value;

            break;
        }
    }

    if (module.install == NULL
    ||  module.destroy == NULL
    ||  module.probe == NULL)
    {
        log("Module `%s` does not implement required functions.", file->name);
        return NULL;
    }

    // Load relocation sections.
    for (size_t i = 0; i < ehdr.e_shnum; i++)
    {
        Elf64_Shdr *section = &shdr[i];

        if (section->sh_type != SHT_RELA)
            continue;

        Elf64_Rela rela_entries[section->sh_size / section->sh_entsize];
        file->file_ops->read(file, section->sh_offset, rela_entries, section->sh_size);

        for (uint j = 0; j < section->sh_size / section->sh_entsize; j++)
        {
            Elf64_Rela *rela = &rela_entries[j];
            Elf64_Sym  *sym  = (Elf64_Sym*)(symtab + (ELF64_R_SYM(rela->r_info) * symtab_hdr->sh_entsize));

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

    // TODO: clean the actual segments allocated for the module.

    module_t *ret = heap_alloc(sizeof(module_t));
    *ret = module;

    log("Kernel module loaded successfully.");
    return ret;
}
