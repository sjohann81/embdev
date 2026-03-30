ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 32K
}

_stack_top = ORIGIN(RAM) + LENGTH(RAM);
_stack_size  = 2K;
_heap_start = _bss_end;
_heap_end = _stack_top - _stack_size;

SECTIONS
{
    .text : {
        _text = .;
        *(.text.init)
        *(.text*)
        _etext = .;
    } > RAM

    .rodata : {
        _rodata = .;
        *(.rodata*)
        _erodata = .;
    } > RAM

    .data : {
        _data = .;
        *(.data*)
        *(.sdata*)
        _edata = .;
    } > RAM

    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        _bss_end = .;
    } > RAM
}
