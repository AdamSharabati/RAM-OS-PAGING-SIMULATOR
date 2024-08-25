#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "mem_sim.h"


int MMPageIndex = 0;
int checkSwapData = 0;
int FrameToPage[MEMORY_SIZE/PAGE_SIZE];

sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_size, int bss_heap_stack_size)
{
    sim_database* sim_DB = (sim_database*) malloc(sizeof(sim_database));
    if (sim_DB == NULL) {
        perror("malloc failed");
        return NULL;
    }

    sim_DB->program_fd = open(exe_file_name, O_RDONLY);
    if(sim_DB->program_fd < 0)
    {
        perror("FILE DOES NOT EXISTS");
        free(sim_DB);
        return NULL;
    }

    sim_DB->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if(sim_DB->swapfile_fd < 0)
    {
        perror("OPEN");
        close(sim_DB->program_fd);
        //free(sim_DB);
        return NULL;
    }

    sim_DB->text_size = text_size;
    sim_DB->data_size = data_size;
    sim_DB->bss_heap_stack_size = bss_heap_stack_size;
    for(int i = 0; i<MEMORY_SIZE;i++)
    {
        sim_DB->main_memory[i] = '0';
    }

    char zeros[201];
    memset(zeros, '0', 200);
    zeros[200] = '\0';
    ssize_t bytes_written = write(sim_DB->swapfile_fd, zeros, 200);

    if (bytes_written < 0) {
        fprintf(stderr, "INITIALIZING SWAP FILE\n");
        close(sim_DB->swapfile_fd);
        close(sim_DB->program_fd);
        return NULL;
    }

    for(int i = 0; i < NUM_OF_PAGES; i++)
    {
        if(i<text_size/PAGE_SIZE)
        {
            sim_DB->page_table[i].P = 1;
        }
        else
        {
            sim_DB->page_table[i].P = 0;
        }
        sim_DB->page_table[i].D = 0;
        sim_DB->page_table[i].V = 0;
        sim_DB->page_table[i].frame_swap = -1;
    }

    for(int i = 0; i<MEMORY_SIZE/PAGE_SIZE;i++)
    {
        FrameToPage[i] = -1;
    }

    return sim_DB;
}

void clear_system(struct sim_database * mem_sim)
{
    close(mem_sim->swapfile_fd);
    close(mem_sim->program_fd);
    free(mem_sim);
}

void getAddress(int address,int* pageAddress, int* offset)
{
    *pageAddress = (address >> 3) & 0x3F;
    *offset = address & 0x07;
}

int swap_page(sim_database *db, int page_index) {
    if (page_index < 0 || page_index >= (MEMORY_SIZE / PAGE_SIZE)) {
        fprintf(stderr, "Invalid page index\n");
        return -1;
    }

    // Find the first fit of 8 consecutive zeros in the swap file
    lseek(db->swapfile_fd, 0, SEEK_SET);
    char buffer[PAGE_SIZE] = {0};
    int swap_index = -1;

    for (int i = 0; i <= SWAP_SIZE - PAGE_SIZE; i += PAGE_SIZE) {
        read(db->swapfile_fd, buffer, PAGE_SIZE);
        int is_empty = 1;
        for (int j = 0; j < PAGE_SIZE; j++) {
            if (buffer[j] != '0') {
                is_empty = 0;
                break;
            }
        }
        if (is_empty) {
            swap_index = i;
            break;
        }
    }

    if (swap_index == -1) {
        fprintf(stderr, "No space in swap file\n");
        return -1;
    }

    // Swap the page from main memory to swap file
    char temp[PAGE_SIZE];
    memcpy(temp, &db->main_memory[page_index * PAGE_SIZE], PAGE_SIZE);
    lseek(db->swapfile_fd, swap_index, SEEK_SET);
    write(db->swapfile_fd, temp, PAGE_SIZE);

    // Swap the zeros to main memory
    memset(temp, '0', PAGE_SIZE);
    memcpy(&db->main_memory[page_index * PAGE_SIZE], temp, PAGE_SIZE);

    return swap_index / PAGE_SIZE;
}

void load_page_from_program(struct sim_database *db, int program_page_address, int main_memory_page_index) {
    if (main_memory_page_index < 0 || main_memory_page_index >= MEMORY_SIZE / PAGE_SIZE) {
        fprintf(stderr, "invalid main memory page index\n");
        return;
    }

    // Calculate the offset in the program file
    off_t offset = program_page_address * PAGE_SIZE;

    // Seek to the correct position in the file
    if (lseek(db->program_fd, offset, SEEK_SET) == -1) {
        perror("lseek failed");
        return;
    }

    // Read the page data from the program file
    char buffer[PAGE_SIZE];
    ssize_t bytes_read = read(db->program_fd, buffer, PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        perror("read failed");
        return;
    }

    // Copy
    memcpy(&db->main_memory[main_memory_page_index * PAGE_SIZE], buffer, PAGE_SIZE);
}

void LMToMM(struct sim_database * mem_sim , int pageAddress)
{
    if(checkSwapData == 1)
    {
        if(mem_sim->page_table[FrameToPage[MMPageIndex]].D == 0)
        {
            for(int i = 0; i<PAGE_SIZE; i++)
            {
                mem_sim->main_memory[MMPageIndex*PAGE_SIZE +i] = '0';
            }
            mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
            mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = -1;
        }
        else
        {
            mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
            mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = swap_page(mem_sim,MMPageIndex);
        }
    }
    FrameToPage[MMPageIndex] = pageAddress;
    load_page_from_program(mem_sim,pageAddress,MMPageIndex);
    mem_sim->page_table[pageAddress].V = 1;
    mem_sim->page_table[pageAddress].frame_swap = MMPageIndex;
    MMPageIndex++;
    if(MMPageIndex == MEMORY_SIZE/PAGE_SIZE)
    {
        MMPageIndex=0;
        checkSwapData = 1;
    }
}

void swap_from_swapfile_to_main(sim_database *db, int swapfile_page_index, int mem_address) {
    if (swapfile_page_index < 0 || mem_address < 0 || mem_address >= (MEMORY_SIZE - PAGE_SIZE)) {
        fprintf(stderr, "Invalid page index or memory address\n");
        return;
    }

    // Seek to the correct position in the swap file
    off_t offset = swapfile_page_index * PAGE_SIZE;
    if (lseek(db->swapfile_fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Failed to seek in swap file");
        return;
    }

    // Read the page from the swap file
    char swap_buffer[PAGE_SIZE];
    ssize_t bytes_read = read(db->swapfile_fd, swap_buffer, PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        fprintf(stderr, "Failed to read the complete page from the swap file\n");
        return;
    }

    // Swap the page from the main memory to the swap file
    char main_memory_buffer[PAGE_SIZE];
    memcpy(main_memory_buffer, &db->main_memory[mem_address * PAGE_SIZE], PAGE_SIZE);

    lseek(db->swapfile_fd, offset, SEEK_SET);
    write(db->swapfile_fd, main_memory_buffer, PAGE_SIZE);

    // Write the page from the swap file to the specified address in the main memory
    memcpy(&db->main_memory[mem_address*PAGE_SIZE], swap_buffer, PAGE_SIZE);
}

void SWToMM(struct sim_database * mem_sim , int SWAddress, int LAddress)
{
    if(checkSwapData == 1)
    {
        if(mem_sim->page_table[FrameToPage[MMPageIndex]].D == 0)
        {
            for(int i = 0; i<PAGE_SIZE; i++)
            {
                mem_sim->main_memory[MMPageIndex*PAGE_SIZE +i] = '0';
            }
            mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
            mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = -1;
        }
        else
        {
            mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
            mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = swap_page(mem_sim,MMPageIndex);
        }
    }

    swap_from_swapfile_to_main(mem_sim,SWAddress, MMPageIndex);
    FrameToPage[MMPageIndex] = LAddress;
    mem_sim->page_table[LAddress].V = 1;
    mem_sim->page_table[LAddress].frame_swap = MMPageIndex;
    MMPageIndex++;
    if(MMPageIndex == MEMORY_SIZE/PAGE_SIZE)
    {
        MMPageIndex=0;
        checkSwapData = 1;
    }
}

void alcPage(struct sim_database * mem_sim, int pageAddress)
{
    if(checkSwapData == 1) {
        if (mem_sim->page_table[FrameToPage[MMPageIndex]].D == 1) {
            mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
            mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = swap_page(mem_sim, MMPageIndex);
        }
    }
    for (int i = 0; i < PAGE_SIZE; i++) {
        mem_sim->main_memory[MMPageIndex * PAGE_SIZE + i] = '0';
    }
    if(FrameToPage[MMPageIndex] != -1 && mem_sim->page_table[FrameToPage[MMPageIndex]].D != 1)
    {
        mem_sim->page_table[FrameToPage[MMPageIndex]].V = 0;
        mem_sim->page_table[FrameToPage[MMPageIndex]].frame_swap = -1;
    }
    FrameToPage[MMPageIndex] = pageAddress;
    mem_sim->page_table[pageAddress].V = 1;
    mem_sim->page_table[pageAddress].frame_swap = MMPageIndex;
    MMPageIndex++;
    if(MMPageIndex == MEMORY_SIZE/PAGE_SIZE)
    {
        MMPageIndex=0;
        checkSwapData = 1;
    }
}

char load (struct sim_database * mem_sim , int address)
{
    int TotalPagesInProgramFile = (mem_sim->text_size + mem_sim->data_size)/PAGE_SIZE;
    int pageAddress;
    int offset;
    getAddress(address,&pageAddress,&offset);
    if(pageAddress > TotalPagesInProgramFile + (mem_sim->bss_heap_stack_size / PAGE_SIZE))
    {
        fprintf(stderr, "INVALID ADDRESS, MAX PAGE = %d\n",TotalPagesInProgramFile + (mem_sim->bss_heap_stack_size / PAGE_SIZE));
        return '\0';
    }
    if(mem_sim->page_table[pageAddress].V == 1)    /////   already in main mem
    {
        int frame = mem_sim->page_table[pageAddress].frame_swap;
        return mem_sim->main_memory[frame*PAGE_SIZE + offset];
    }
    else
    {
        if(mem_sim->page_table[pageAddress].P == 1)   /////     TEXT
        {
            LMToMM(mem_sim,pageAddress);
            int frame = mem_sim->page_table[pageAddress].frame_swap;
            return mem_sim->main_memory[frame*PAGE_SIZE + offset];
        }
        else
        {
            if(mem_sim->page_table[pageAddress].D == 1)  ///    take from swap
            {
                int SWAddress = mem_sim->page_table[pageAddress].frame_swap;
                SWToMM(mem_sim,SWAddress,pageAddress);
                int frame = mem_sim->page_table[pageAddress].frame_swap;
                return mem_sim->main_memory[frame*PAGE_SIZE + offset];
            }
            else    //////     check in Logical Memory   OR allocate new page.
            {
                if(pageAddress<TotalPagesInProgramFile)
                {
                    LMToMM(mem_sim,pageAddress);
                    int frame = mem_sim->page_table[pageAddress].frame_swap;
                    return mem_sim->main_memory[frame*PAGE_SIZE + offset];
                }
                else
                {
                    alcPage(mem_sim,pageAddress);
                    int frame = mem_sim->page_table[pageAddress].frame_swap;
                    return mem_sim->main_memory[frame*PAGE_SIZE + offset];
                }
            }
        }
    }
}

void store(struct sim_database * mem_sim , int address, char value)
{
    int pageAddress;
    int offset;
    getAddress(address,&pageAddress,&offset);
    if(mem_sim->page_table[pageAddress].P == 1)
    {
        fprintf(stderr, "INVALID ADDRESS, CANT WRITE TO TEXT PAGE\n");
        return;
    }
    char result = load(mem_sim,address);

    if(result == '\0')
    {
        return;
    }

    int frame = mem_sim->page_table[pageAddress].frame_swap;
    mem_sim->main_memory[frame*PAGE_SIZE + offset] = value;
    mem_sim->page_table[pageAddress].D = 1;
}

/**************************************************************************************/
void print_memory(sim_database* mem_sim) {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", mem_sim->main_memory[i]);
    }
}
/************************************************************************************/
void print_swap(sim_database* mem_sim) {
    char str[PAGE_SIZE];
    int i;
    printf("\n Swap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
        for(i = 0; i < PAGE_SIZE; i++) {
            printf("[%c]\t", str[i]);
        }
        printf("\n");
    }
}
/***************************************************************************************/
void print_page_table(sim_database* mem_sim) {
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame_swap\n");
    for (i = 0; i < NUM_OF_PAGES; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V,
               mem_sim->page_table[i].D,
               mem_sim->page_table[i].P, mem_sim->page_table[i].frame_swap);
    }
}
