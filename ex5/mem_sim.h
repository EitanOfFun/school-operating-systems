#include <stdbool.h>
#ifndef MEM_SIM_H
#define MEM_SIM_H

#define PAGE_SIZE 5
#define NUM_OF_PAGES 25
#define MEMORY_SIZE 20

typedef struct page_descriptor {
    unsigned int V;
    unsigned int D;
    unsigned int P;
    int frame;
} page_descriptor;

typedef struct sim_database {
    page_descriptor page_table[NUM_OF_PAGES];
    char main_memory[MEMORY_SIZE];
    int swapfile_fd;
    int program_fd;
    int text_size;
    int data_bss_size;
    int heap_stack_size;
} sim_database;
typedef struct frame_manager {
    ssize_t current_frame;
    int frame_to_page[MEMORY_SIZE / PAGE_SIZE];
} frame_manager;

sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_bss_size, int heap_stack_size);
char load (sim_database * mem_sim , int address);
void store(sim_database * mem_sim , int address, char value);
void print_swap (sim_database * mem_sim);
void print_page_table(sim_database * mem_sim);
void clear_system(sim_database * mem_sim);
void print_memory(sim_database * mem_sim);

ssize_t get_next_frame(frame_manager* frameManager);
void error(const char* msg);
bool is_address_invalid(sim_database *mem_sim, int address);
bool is_data_bss(sim_database *mem_sim, int page);
bool copy_page_from_file(int fd, int page, char *page_data);
bool copy_page_to_file(int fd, int page, char *page_data);
bool init_new_page(char* page_data);

#endif //MEM_SIM_H
