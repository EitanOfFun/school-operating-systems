#include<stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "mem_sim.h"

int main() {
    char val;
    sim_database* mem_sim = init_system ("exec_file", "swap_file" ,25, 50, 50);
    val = load (mem_sim , 64); // page 12
    val = load (mem_sim , 66); // page 13
    val = load (mem_sim , 2); // page 0
    store(mem_sim , 98,'X'); // page 19
    val = load (mem_sim ,16); // page 3
    val = load (mem_sim ,70); // page 14
    store(mem_sim ,32,'Y'); // page 6
    store (mem_sim ,15,'Z'); // page 3 - CAN'T WRITE!
    val = load (mem_sim ,23); // page 4
    print_memory(mem_sim);
    print_swap(mem_sim);
    clear_system(mem_sim);

    return (EXIT_SUCCESS);
}