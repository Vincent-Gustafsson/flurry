#pragma once

#include <limine.h>
#include <flurry/system/boot.h>

typedef enum {
    ELF_RESULT_OK,
    ELF_RESULT_ERR_NOT_ELF,
    ELF_RESULT_ERR_INVALID_MACHINE,
    ELF_RESULT_ERR_INVALID_CLASS,
    ELF_RESULT_ERR_INVALID_ENCODING,
    ELF_RESULT_ERR_INVALID_VERSION,
} ElfResult;

typedef struct {
    void* entry_point;
    uint64_t phdrs_size;
    uint64_t phdrs_offset;
    uint64_t phdrs_count;
} ElfFile;

ElfResult elf_read(Module* module, ElfFile* file);
