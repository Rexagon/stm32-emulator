MEMORY
{
  FLASH : ORIGIN = 0x00000000, LENGTH = 64K
  RAM : ORIGIN = 0x20000000, LENGTH = 20K
}

_stack_start = ORIGIN(RAM) + LENGTH(RAM);

