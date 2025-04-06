#include "flurry/abi/sysv/elf.h"

#include "log.h"
#include "stdint.h"

/* Types and structures taken from https://uclibc.org/docs/elf-64-gen.pdf */

/* ELF File Types */
#define ET_NONE   0   // No file type
#define ET_REL    1   // Relocatable file
#define ET_EXEC   2   // Executable file
#define ET_DYN    3   // Shared object file
#define ET_CORE   4   // Core file

/* ELF Machine Types */
#define EM_NONE     0   // No machine
#define EM_X86_64   62  // AMD x86-64 architecture

/* ELF Version */
#define EV_NONE     0   // Invalid version
#define EV_CURRENT  1   // Current version

/* ELF Classes */
#define ELFCLASSNONE  0
#define ELFCLASS32    1
#define ELFCLASS64    2

/* Data Encodings */
#define ELFDATANONE   0
#define ELFDATA2LSB   1         // Little-endian
#define ELFDATA2MSB   2         // Big-endian

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

typedef struct {
    unsigned char e_ident[16];  // ELF identification
    Elf64_Half    e_type;       // Object file type
    Elf64_Half    e_machine;    // Machine type
    Elf64_Word    e_version;    // Object file version
    Elf64_Addr    e_entry;      // Entry point address
    Elf64_Off     e_phoff;      // Program header offset
    Elf64_Off     e_shoff;      // Section header offset
    Elf64_Word    e_flags;      // Processor-specific flags
    Elf64_Half    e_ehsize;     // ELF header size
    Elf64_Half    e_phentsize;  // Size of program header entry
    Elf64_Half    e_phnum;      // Number of program header entries
    Elf64_Half    e_shentsize;  // Size of section header entry
    Elf64_Half    e_shnum;      // Number of section header entries
    Elf64_Half    e_shstrndx;   // Section name string table index
} Elf64_Ehdr;



ElfResult elf_read(Module* module, ElfFile* elf_file) {
    Elf64_Ehdr* header = module->address;
    
    if (header->e_ident[0] != 0x7f || header->e_ident[1] != 'E' || header->e_ident[2] != 'L' || header->e_ident[3] != 'F')
        return ELF_RESULT_ERR_NOT_ELF;

    if (header->e_ident[4] != ELFCLASS64)
        return ELF_RESULT_ERR_INVALID_CLASS;

    if (header->e_machine != EM_X86_64)
        return ELF_RESULT_ERR_INVALID_MACHINE;

    if (header->e_ident[5] != ELFDATA2LSB)
        return ELF_RESULT_ERR_INVALID_ENCODING;

    if (header->e_version != EV_CURRENT)
        return ELF_RESULT_ERR_INVALID_VERSION;

    *elf_file = (ElfFile) {
        .entry_point = (void*) header->e_entry,
        .phdrs_size = header->e_phentsize,
        .phdrs_offset = header->e_phoff,
        .phdrs_count = header->e_phnum
    };
    
    return ELF_RESULT_OK;
}
