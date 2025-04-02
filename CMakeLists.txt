cmake_minimum_required(VERSION 3.22)
project(flurry_os C ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "-std=c11")

set(CMAKE_ASM_NASM_FLAGS "-f elf64")


# Add your NASM source files
set(ASM_SOURCES
    ../kernel/cpu/gdt/gdt.asm
    ../kernel/interrupts/isr.asm
    ../kernel/multitasking/sched.asm
)

# Compile NASM files
foreach(ASM_FILE IN LISTS ASM_SOURCES)
    get_filename_component(ASM_OUTPUT ${ASM_FILE} NAME_WE)
    add_custom_command(
            OUTPUT ${ASM_OUTPUT}.o
            COMMAND ${CMAKE_ASM_NASM_COMPILER} ${CMAKE_ASM_NASM_FLAGS} -o ${ASM_OUTPUT}.o ${ASM_FILE}
            DEPENDS ${ASM_FILE}
    )
    list(APPEND ASM_OBJECTS ${ASM_OUTPUT}.o)
endforeach()


# https://github.com/sbwtw/sos/blob/master/CMakeLists.txt

include(uacpi/uacpi.cmake)
include_directories(
        include
        ${UACPI_INCLUDES}
)

# binary
add_executable(flurry_os.bin
        log/tty.c
        kernel/main.c
        kernel/string.c
        kernel/cpu/gdt/gdt.c
        kernel/cpu/idt/idt.c
        kernel/interrupts/interrupts.c
        kernel/memory/pmm.c
        kernel/memory/vmm.c
        kernel/memory/paging.c
        kernel/boot/entry.c
        kernel/common.c
        kernel/memory/kmalloc.c
        kernel/acpi/uacpi_interface.c
        ${UACPI_SOURCES}
        kernel/time/timer.c
        kernel/acpi/hpet.c
        kernel/acpi/madt.c
        kernel/hardware/lapic.c
        include/flurry/hardware/lapic.h
        include/flurry/hardware/lapic.h
        kernel/hardware/ioapic.c
        include/flurry/hardware/ioapic.h
        kernel/multitasking/process.c
        include/flurry/multitasking/process.h
        kernel/multitasking/thread.c
        include/flurry/multitasking/thread.h
        include/flurry/string.h
        kernel/multitasking/sched.c
        include/flurry/multitasking/sched.h
        log.c
        include/log.h
        kernel/multitasking/events.c
        include/flurry/multitasking/events.h
        kernel/hardware/tsc.c
        include/flurry/hardware/tsc.h
        include/flurry/cpu/msr.h
)

# Link NASM objects with your main binary
target_sources(flurry_os.bin PRIVATE ${ASM_OBJECTS})


target_compile_options(flurry_os.bin PRIVATE
    -Wall
    -Wextra
    -std=gnu11
    -ffreestanding
    -fno-stack-protector
    -fno-stack-check
    -fno-PIC
    -m64
    -march=x86-64
    -mno-80387
    -mno-mmx
    -mno-sse
    -mno-sse2
    -mno-red-zone
    -mcmodel=kernel
    -g
    -Iinclude/
)

target_link_options(flurry_os.bin PRIVATE
    -Wl,-m,elf_x86_64
    -Wl,--build-id=none
    -nostdlib
    -static
    -z max-page-size=0x1000
    -T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding
)



# iso
add_custom_target(
    flurry_os_iso
    COMMAND ../generate_iso.sh > out.log 2> err.log
    COMMENT "Generating the kernel bootable iso file"
    DEPENDS flurry_os.bin
    VERBATIM
)
