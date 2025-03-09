SECTION .data               ; Data section
    gdtr:
        DW 0                 ; For limit storage (2 bytes)
        DQ 0                 ; For base storage (8 bytes)

SECTION .text               ; Code section
    global set_gdt          ; Expose set_gdt function to linker
    global reload_segments  ; Expose reload_segments function to linker
    global set_tss

set_gdt:
   MOV   [gdtr], DI
   MOV   [gdtr+2], RSI
   LGDT  [gdtr]
   RET            ; Return from the function

set_tss:
    MOV AX, 0x28
    LTR AX
    RET

reload_segments:
   ; Reload CS register:
   PUSH 0x08                 ; Push code segment to stack, 0x08 is a stand-in for your code segment
   LEA RAX, [rel .reload_CS] ; Load address of .reload_CS into RAX
   PUSH RAX                  ; Push this value to the stack
   RETFQ                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.reload_CS:
   ; Reload data segment registers
   MOV   AX, 0x10 ; 0x10 is a stand-in for your data segment
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET
