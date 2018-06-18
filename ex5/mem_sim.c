#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "mem_sim.h"

frame_manager fm; // global frame manager

struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size, int data_bss_size, int heap_stack_size) {
    struct sim_database* mem_sim = (struct sim_database*)malloc(sizeof(struct sim_database*));
    mem_sim->text_size = text_size;
    mem_sim->data_bss_size = data_bss_size;
    mem_sim->heap_stack_size = heap_stack_size;
    mem_sim->program_fd = open(exe_file_name, O_RDONLY);

    if (mem_sim->program_fd != -1) {
        mem_sim->swapfile_fd = open(swap_file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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
        fm.frame_to_page[i] = -1;

    return mem_sim;
}
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
void store(struct sim_database* mem_sim, int address, char value) {
    const int page = address / PAGE_SIZE;
    const int offset = address % PAGE_SIZE;

    if (is_address_invalid(mem_sim, address)) {
        error("Invalid Address!\n");
        return;
    }
    if (mem_sim->page_table[page].P == 0) {
        error("Cannot write over code!\n");
        return;
    }
    if (mem_sim->page_table[page].V != 1)
        load(mem_sim, address);
    mem_sim->main_memory[mem_sim->page_table[page].frame * PAGE_SIZE + offset] = value;
    mem_sim->page_table[page].D = 1;
}
ssize_t get_next_frame(frame_manager* frameManager) {
    return (frameManager->current_frame + 1) % (MEMORY_SIZE / PAGE_SIZE);
}
void error(const char* msg) {
    fprintf(stderr, "%s", msg);
    fflush(stderr);
}
bool is_address_invalid(struct sim_database *mem_sim, int address) {
    return address < 0 || address >= mem_sim->text_size + mem_sim->data_bss_size + mem_sim->heap_stack_size;
}
bool is_data_bss(struct sim_database *mem_sim, int page) {
    return page >= mem_sim->text_size && page < mem_sim->text_size + mem_sim->data_bss_size;
}
bool copy_page_from_file(int fd, int page, char *page_data) {
    lseek(fd, page * PAGE_SIZE, SEEK_SET);

    if (read(fd, page_data, PAGE_SIZE) != PAGE_SIZE) {
        error("Read from file failed!\n");
        return false;
    }
    return true;
}
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
