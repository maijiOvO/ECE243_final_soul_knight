#include "random.h"

static unsigned int rng_state = 1;

void my_srand(unsigned int seed) {
    if (seed == 0) seed = 1;
    rng_state = seed;
}

unsigned int my_rand(void) {
    rng_state = rng_state * 1103515245u + 12345u;
    return (rng_state >> 16) & 0x7FFFu;
}