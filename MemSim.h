/*
 * MemSim.h
 *
 * Header file for simulator for CSCI 447 memory management exercise
 *
 * Andrew Cox
 * 3 June 2024
 */

#ifndef MEMSIM_H
#define MEMSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// global variables
char* algorithm = NULL;                     // page replacement algorithm
int readFaults = 0;                         // read faults
int writeFaults = 0;                        // write (and read) faults
int processes = 0;                          // process counter
int frequency = 0;                          // page frequency counter
int startIndex = 0;                         // index after last eviction (LRU)

// define constants
#define PAGE_SIZE 4096                      // 4KB
#define MEM_SIZE 8388608                    // 8MB
#define ADDR_SIZE 32                        // 32-bit address space
#define MAX_FRAMES (MEM_SIZE / PAGE_SIZE)   // 2048
#define MAX_PROCESSES 64
#define OFFSET_BITS 12                      // offset for bit shifting
#define PAGE_BITS (ADDR_SIZE - OFFSET_BITS) // 20

// define page table entry
typedef struct process {
    int pid;
    int page;
    int reference;
    int frequency;
    int dirty;
    int valid;
} process;

// prototypes and declarations
int find(int pid, int page);                               // find page in page table
int Replace(int pid, int page);                            // page replacement
int LRU(int pid, int page);                                // LRU page replacement
int LFU(int pid, int page);                                // LFU page replacement
int Random(int pid, int page);                             // random page replacement
int processActive(int pid);                                // check if process is active
void updateEntry(int frame, int pid, int page, int write); // update page table entry
void updateFaults(int frame);                              // update fault counters
void decay();                                              // decay frequency counter
int Access(int pid, int address, int write);               // access memory
void Terminate(int pid);                                   // terminate process
process table[MAX_FRAMES];                                 // page table
process activeProcesses[MAX_PROCESSES] = {0};              // active processes
void *runAnimation(void *arg);                             // run animation
void stopAnimation();                                      // stop animation
void Simulate(int rounds);                                 // run simulation

/*
 * *********************************************************************
 * This is just a fun animation to show while the program is running
 */
int animationRunning = 1; // flag to control animation
int MAXDOTS = 30;         // maximum number of dots in animation

void* runAnimation(void* arg) {
    int dotCount = 0;

    while (animationRunning) {
        printf("\rSimulation running [");
        for (int i = 0; i < dotCount; i++) {
            printf(".");
        }
        for (int i = dotCount; i < MAXDOTS; i++) {
            printf(" ");
        }
        printf("]");
        fflush(stdout);

        dotCount = (dotCount + 1) % MAXDOTS;
        usleep(100000); // Delay for 100 milliseconds
    }

    // Clear the animation line
    printf("\r%*s\r", MAXDOTS + 22, "");
    fflush(stdout);

    return NULL;
}

void stopAnimation() {
    animationRunning = 0;
    // Clear the animation line
    printf("\r%*s\r",MAXDOTS + 22, "");
    fflush(stdout);
}
/*
 * *********************************************************************
 */

#endif
