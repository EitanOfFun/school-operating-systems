#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "mem_sim.h"

frame_manager fm; // global frame manager

/**
 * Initializes the system
 * @param exe_file_name executable file name
 * @param swap_file_name swap file name to create
 * @param text_size size in bytes of program's instructions
 * @param data_bss_size size in bytes of program's data / bss
 * @param heap_stack_size size in bytes of program's heap
 * @return
 */
struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_bss_size, int heap_stack_size) {
    struct sim_database* mem_sim = (struct sim_database*)malloc(sizeof(struct sim_database*));
    mem_sim->text_size = text_size;
    mem_sim->data_bss_size = data_bss_size;
    mem_sim->heap_stack_size = heap_stack_size;
    mem_sim->program_fd = open(exe_file_name, O_RDONLY);

    if (mem_sim->program_fd != -1) {
        mem_sim->swapfile_fd = open(swap_file_name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (mem_sim->swapfile_fd != 1) {
            for (int i = 0; i < MEMORY_SIZE; ++i)
                mem_sim->main_memory[i] = '0';

            for (int i = 0; i < NUM_OF_PAGES; ++i) {
                mem_sim->page_table[i].frame = -1;
                mem_sim->page_table[i].V = 0;
                mem_sim->page_table[i].D = 0;
                if (i < text_size / PAGE_SIZE)
                    mem_sim->page_table[i].P = 0;
                else
                    mem_sim->page_table[i].P = 1;
            }
        } else {
            error("NULL in swapfile");
        }
    } else {
        free(mem_sim);
        error("exec file doesn't exist!\n");
        exit(EXIT_FAILURE);
    }
    // initialize global frame manager
    fm.current_frame = 0;
    for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; ++i)
        fm.frame_to_page[i] = -1; // set all pages to -1 (aka no page stored in frame i)

    return mem_sim;
}
/**
 * loads the page from page table and into RAM (if it's not there already)
 * and returns the char at the address given
 * @param mem_sim simulated memory
 * @param address to load
 * @return char at address
 */
char load(struct sim_database *mem_sim, int address) {
    const int page = address / PAGE_SIZE;
    const int offset = address % PAGE_SIZE;
    const int main_memory_index = mem_sim->page_table[page].frame * PAGE_SIZE + offset;

    if (is_address_invalid(mem_sim, address)) {
        error("Invalid Address!\n");
        return '\0';
    }
    if (mem_sim->page_table[page].V == 1) {
        if (main_memory_index < 0 || main_memory_index >= MEMORY_SIZE) {
            error("Invalid address!\n");
            return '\0';
        }
        return mem_sim->main_memory[main_memory_index];
    } else {
        char page_data[PAGE_SIZE];

        if (mem_sim->page_table[page].P == 0)
            copy_page_from_file(mem_sim->program_fd, page, page_data);
        else if (mem_sim->page_table[page].D == 1)
            copy_page_from_file(mem_sim->swapfile_fd, page, page_data);
        else if (is_data_bss(mem_sim, page))
            copy_page_from_file(mem_sim->program_fd, page, page_data);
        else
            init_new_page(page_data);

        const int old_page = fm.frame_to_page[fm.current_frame];

        if (old_page != -1 && mem_sim->page_table[old_page].D == 1) {
            lseek(mem_sim->swapfile_fd, old_page * PAGE_SIZE, SEEK_SET);
            if (write(mem_sim->swapfile_fd, mem_sim->main_memory + mem_sim->page_table[old_page].frame * PAGE_SIZE, PAGE_SIZE) < PAGE_SIZE) {
                error("Failed to write to swap file!\n");
                return '\0';
            }
        }

        mem_sim->page_table[old_page].V = 0;
        mem_sim->page_table[old_page].frame = -1;

        for (int i = 0; i < PAGE_SIZE; ++i)
            mem_sim->main_memory[fm.current_frame * PAGE_SIZE + i] = page_data[i];

        const ssize_t index = fm.current_frame * PAGE_SIZE + offset;

        mem_sim->page_table[page].V = 1;
        mem_sim->page_table[page].frame = fm.current_frame;

        fm.frame_to_page[fm.current_frame] = page;
        fm.current_frame = get_next_frame(&fm);

        return mem_sim->main_memory[index];
    }
}
/**
 * loads the page from page table and into RAM (if it's not there already)
 * and writes the value at the address given
 * @param mem_sim simulated memory
 * @param address to store value in
 * @param value to store at address location
 */
void store(struct sim_database* mem_sim, int address, char value) {
    const int page = address / PAGE_SIZE;
    const int offset = address % PAGE_SIZE;

    if (is_address_invalid(mem_sim, address)) { // not a valid address (too high or negative)
        error("Invalid Address!\n");
        return;
    }
    if (mem_sim->page_table[page].P == 0) { // cannot write to text area addresses
        error("Access Denied!\n");
        return;
    }
    if (mem_sim->page_table[page].V != 1)
        load(mem_sim, address); // use load to load addresses page into memory
    mem_sim->main_memory[mem_sim->page_table[page].frame * PAGE_SIZE + offset] = value; // set value in main_memory at specified address to new value
    mem_sim->page_table[page].D = 1; // after setting value, set dirty bit to 1
}
/**
 * helper function to get next available frame from frame manager (loops over array with modulo of array length)
 * e.g. if number of frames in memory is 4 then:
 * current frame will loop from 0 1 2 3 0 1 2 3 0 ... and so on
 * this function returns the next index in this series.
 * e.g. if current frame is 3 from previous example: this function return 0
 *      if current frame is 0 from previous example: this function return 1
 *  ... and so on
 * @param frameManager
 * @return
 */
ssize_t get_next_frame(frame_manager* frameManager) {
    return (frameManager->current_frame + 1) % (MEMORY_SIZE / PAGE_SIZE);
}
/**
 * helper function that prints msg to stderr and flushes stderr
 * @param msg
 */
void error(const char* msg) {
    fprintf(stderr, "%s", msg);
    fflush(stderr);
}
/**
 * checks bounds of address (not negative and within bounds of processes total memory address
 * @param mem_sim
 * @param address
 * @return true if address in out of bounds
 */
bool is_address_invalid(struct sim_database *mem_sim, int address) {
    return address < 0 || address >= mem_sim->text_size + mem_sim->data_bss_size + mem_sim->heap_stack_size;
}
/**
 * helper function that checks if page is in data / bss section (above text but under heap section)
 * @param mem_sim
 * @param page
 * @return
 */
bool is_data_bss(struct sim_database *mem_sim, int page) {
    return page * PAGE_SIZE >= mem_sim->text_size && page * PAGE_SIZE < mem_sim->text_size + mem_sim->data_bss_size;
}
bool copy_page_from_file(int fd, int page, char *page_data) {
    lseek(fd, page * PAGE_SIZE, SEEK_SET);

    if (read(fd, page_data, PAGE_SIZE) != PAGE_SIZE) {
        error("Read from file failed!\n");
        return false;
    }
    return true;
}
/**
 * initialized data (char array) full of zeros (PAGE_SIZE 0's)
 * @param data
 */
void init_new_page(char* data) {
    for (int i = 0; i < PAGE_SIZE; ++i)
        data[i] = '0';
}
void print_memory(struct sim_database * mem_sim) {
    printf("\n Physical memory\n");
    for(int i = 0; i < MEMORY_SIZE; i++)
        printf("[%c]\n", mem_sim->main_memory[i]);
}
void print_swap (struct sim_database * mem_sim) {
    char str[PAGE_SIZE];
    printf("\n Swap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
        for(int i = 0; i < PAGE_SIZE; i++)
            printf("[%c]\t", str[i]);
        printf("\n");
    }
}
void print_page_table(struct sim_database * mem_sim) {
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame\n");
    for (int i = 0; i < NUM_OF_PAGES; i++)
        printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V,
               mem_sim->page_table[i].D,
               mem_sim->page_table[i].P, mem_sim->page_table[i].frame);
}
void clear_system(struct sim_database * mem_sim) {
    close(mem_sim->program_fd);
    close(mem_sim->swapfile_fd);
    free(mem_sim);
}
