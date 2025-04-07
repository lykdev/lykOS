#include "elf.h"

#include <common/log.h>
#include <lib/string.h>

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