ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 32K
}

_stack_top = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text : {
        *(.text.init)
        *(.text*)
    } > RAM

    .rodata : {
        *(.rodata*)
    } > RAM

    .data (NOLOAD) : {
        *(.data*)
    } > RAM

    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > RAM
}
