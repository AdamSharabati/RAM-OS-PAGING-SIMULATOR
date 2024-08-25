#include <stdio.h>
#include <string.h>
#include "mem_sim.h"

#define EXE_FILE "program"
#define SWAP_FILE "swap"


int main() {
    // Initialize the system
    int text_size = 16; // Example sizes
    int data_size = 32;
    int bss_heap_stack_size = 40;
    sim_database *sim_db = init_system(EXE_FILE, SWAP_FILE, text_size, data_size, bss_heap_stack_size);

    if (sim_db == NULL) {
        fprintf(stderr, "Failed to initialize system\n");
        return 1;
    }

    // Perform some operations
    printf("Loading address 0...\n");
    char val = load(sim_db, 0);
    printf("Value at address 0: %c\n", val);

    printf("Storing value 'B' at address 9...\n");
    store(sim_db, 9, 'B');

    printf("Loading address 8...\n");
    val = load(sim_db, 8);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 16...\n");
    val = load(sim_db, 16);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 24...\n");
    val = load(sim_db, 24);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 32...\n");
    val = load(sim_db, 32);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 40...\n");
    val = load(sim_db, 40);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 0...\n");
    val = load(sim_db, 0);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 8...\n");
    val = load(sim_db, 8);
    printf("Value at address 8: %c\n", val);

    printf("Storing value 'X' at address 17...\n");
    store(sim_db, 17, 'X');

    printf("Loading address 24...\n");
    val = load(sim_db, 24);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 32...\n");
    val = load(sim_db, 32);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 40...\n");
    val = load(sim_db, 40);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 0...\n");
    val = load(sim_db, 0);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 8...\n");
    val = load(sim_db, 8);
    printf("Value at address 8: %c\n", val);

    printf("Storing value 'X' at address 24...\n");
    store(sim_db, 25, 'X');

    printf("Loading address 56...\n");
    val = load(sim_db, 56);
    printf("Value at address 56: %c\n", val);

    printf("Storing value 'X' at address 57...\n");
    store(sim_db, 57, 'X');

    printf("Storing value 'X' at address 56...\n");
    store(sim_db, 56, 'A');

    printf("Storing value 'B' at address 58...\n");
    store(sim_db, 58, 'B');

    printf("Storing value 'C' at address 59...\n");
    store(sim_db, 59, 'C');

    printf("Loading address 16...\n");
    val = load(sim_db, 16);
    printf("Value at address 16: %c\n", val);

    printf("Storing value 'X' at address 41...\n");
    store(sim_db, 41, 'X');

    printf("Loading address 33...\n");
    val = load(sim_db, 33);
    printf("Value at address 33: %c\n", val);


    printf("Loading address 8...\n");
    val = load(sim_db, 8);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 17...\n");
    val = load(sim_db, 17);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 24...\n");
    val = load(sim_db, 24);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 32...\n");
    val = load(sim_db, 32);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 40...\n");
    val = load(sim_db, 40);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 0...\n");
    val = load(sim_db, 0);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 48...\n");
    val = load(sim_db, 48);
    printf("Value at address 0: %c\n", val);

    printf("Loading address 66...\n");
    val = load(sim_db, 66);
    printf("Value at address 66: %c\n", val);

    printf("Storing value 'D' at address 26...\n");
    store(sim_db, 26, 'D');

    printf("Storing value 'B' at address 15...\n");
    store(sim_db, 15, 'B');

    printf("Storing value 'A' at address 14...\n");
    store(sim_db, 14, 'A');

    printf("Storing value 'D' at address 24...\n");
    store(sim_db, 24, 'D');

    printf("Storing value 'C' at address 16...\n");
    store(sim_db, 16, 'C');

    printf("Storing value 'F' at address 16...\n");
    store(sim_db, 32, 'E');

    printf("Storing value 'E' at address 16...\n");
    store(sim_db, 40, 'F');

    load(sim_db,0);
    load(sim_db,8);


    // Print memory, swap, and page table
    print_memory(sim_db);
    print_swap(sim_db);
    print_page_table(sim_db);

    // Cleanup
    clear_system(sim_db);

    return 0;
}
