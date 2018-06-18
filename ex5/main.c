#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <zconf.h>
#include "mem_sim.h"


int main() {
//    int frame_to_page[NUM_OF_PAGES];
//    struct sim_database* db = init_system("kjhjk", "swap_file", 25, 50, 50);
//    print_memory(db);
//    print_page_table(db);
//    print_swap(db);
//    clear_system(db);

    char val;
    struct sim_database* mem_sim = init_system("exec_file", "swap_file" ,25, 50, 50);
//    val = load (mem_sim , 64);
    val = load (mem_sim , 96);
//    val = load (mem_sim , 2);
//    store(mem_sim , 98,'X');
//    val = load (mem_sim ,16);
//    val = load (mem_sim ,70);
//    store(mem_sim ,32,'Y');
//    store (mem_sim ,15,'Z');
//    val = load (mem_sim ,23);
    print_page_table(mem_sim);
    print_memory(mem_sim);
    print_swap(mem_sim);
    clear_system(mem_sim);

    return 0;
}