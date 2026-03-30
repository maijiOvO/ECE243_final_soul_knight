.option norelax
.global _start

_start:
    la gp, __global_pointer$
    li sp, 0x04000000
    call main

1:
    j 1b
    