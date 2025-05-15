/*
 *	PageSim.c
 *
 *	CSCI 447
 *  Spring 2024
 *  Memory Management Simulator
 *
 *	Andrew Cox
 *  26 May 2024
 */

#include "MemSim.h"

// find page in memory
int find(int pid, int page) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (table[i].pid == pid && table[i].page == page && table[i].valid == 1) {
            return i; // page found
        }
    }
    return Replace(pid, page); // page not found
}

// page replacement function
int Replace(int pid, int page) {

    if (strcmp(algorithm, "LRU") == 0) {
        // LRU algorithm
        return LRU(pid, page);
    } else if (strcmp(algorithm, "LFU") == 0) {
        // LFU algorithm
        return LFU(pid, page);
    } else if (strcmp(algorithm, "Random") == 0) {
        // Random algorithm
        return Random(pid, page);
    }
    return -1;
}

// LRU algorithm
// order of priority for replacement (dirty, reference): (0, 0) > (1, 0) > (0, 1) > (1, 1)
int LRU(int pid, int page) {
    // find the least recently used page starting from startIndex

    int lru = startIndex; // index after last eviction
    int pass = 0;         // number of times table has been searched
    int dirty = 0;        // dirty bit
    int reference = 0;    // reference bit
    int found = 0;        // flag for found process

    do {
        if (table[lru].dirty == dirty && table[lru].reference == reference) {
            found = 1;
            break;
        }
        lru = (lru + 1) % MAX_FRAMES;
        if (lru == startIndex) {
            pass++;
            if (pass == 1) { // (1, 0)
                dirty = 1;
                reference = 0;
            } else if (pass == 2) { // (0, 1)
                dirty = 0;
                reference = 1;
            } else if (pass == 3) { // (1, 1)
                dirty = 1;
                reference = 1;
            } else if (pass == 4) {
                return -1;
            }
        }
    } while (!found);

    // update reference bits for start index to evicted index -1
    for (int i = startIndex; i != lru; i = (i + 1) % MAX_FRAMES) {
        table[i].reference = 0;
    }

    // update startIndex
    startIndex = (lru+1) % MAX_FRAMES;

    return lru;
}

// LFU algorithm
int LFU(int pid, int page) {
    // find the least frequently used page
    int lfu = 0;
    for (int i = 1; i < MAX_FRAMES; i++) {
        if (table[i].frequency < table[lfu].frequency) {
            lfu = i;
        }
    }

    // update frequency counter
    table[lfu].frequency++;

    return lfu;
}

// Random algorithm
int Random(int pid, int page) {
    // generate a random number between 0 and MAX_FRAMES
    int random = rand() % MAX_FRAMES;

    return random;
}

// helper function to check if process is active
int processActive(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (activeProcesses[i].pid == pid) {
            return 1;
        }
    }
    return 0;
}

// helper function to update entry in page table
void updateEntry(int frame, int pid, int page, int write) {
    table[frame].pid = pid;
    table[frame].page = page;
    table[frame].reference = 1;
    table[frame].frequency++;
    table[frame].dirty = write;
    table[frame].valid = 1;
}

// helper function to update fault counters
void updateFaults(int frame) {
    if (table[frame].dirty) {
        writeFaults++;
    } else {
        readFaults++;
    }
}

// helper function to decay frequency counters
void decay() {

    frequency++;
    // halve frequency counters every 1000 accesses
    if (frequency >= 1000) {
        for (int j = 0; j < MAX_FRAMES; j++) {
            table[j].frequency /= 2;
        }
        frequency = 0;
    }
}


// called in response to a memory access attempt by process pid to memory address
int Access(int pid, int address, int write) {

    // extract page number from address
    unsigned int page = (address >> OFFSET_BITS);

    // decay frequency counters
    decay();

    // check if process is active
    if (processActive(pid)) {
        for (int i = 0; i < MAX_FRAMES; i++) {
            if (table[i].pid == pid && table[i].page == page) {
                updateEntry(i, pid, page, write);
                return 1; // access granted
            }
        }

        // page fault for active process
        int frame = find(pid, page);
        if (frame == -1) {
            return 0; // no frames available
        }

        // update faults
        updateFaults(frame);

        // update entry
        updateEntry(frame, pid, page, write);

        return 1; // access granted

    } else { // if process is not active

        // check if process can be added
        if (processes >= MAX_PROCESSES) {
            return 0; // deny access
        }

        // add process to active processes
        activeProcesses[processes].pid = pid;
        processes++;

        // Page fault for inactive process
        int frame = find(pid, page);
        if (frame == -1) {
            return 0; // no frames available
        }

        // update faults
        updateFaults(frame);

        // update entry
        updateEntry(frame, pid, page, write);

        return 1; // access granted
    }

    return 0; // default deny access
}

void Terminate(int pid) {
    // Remove the process from the active processes array
    for (int i = 0; i < processes; i++) {
        if (activeProcesses[i].pid == pid) {
            // Shift the remaining processes to fill the gap
            for (int j = i; j < processes - 1; j++) {
                activeProcesses[j] = activeProcesses[j + 1];
            }
            processes--;
            break;
        }
    }

    // update all pages for the process in the inverted page table
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (table[i].pid == pid) {
            table[i].valid = 0;
        }
    }
}

// print page table
void printResults() {
    // <row,PID,p,dirty bit,reference bit>
    for (int i = 0; i < MAX_FRAMES; i++) {
        printf("<Row: %5d, PID: %6d, Page: %6d, Dirty: %2d, Reference: %2d>\n",
               i, table[i].pid, table[i].page, table[i].dirty, table[i].reference);

        table[i].valid = 0;
    }

    // print page faults
    printf("\nRead Faults: %d\nWrite Faults: %d\n", readFaults, writeFaults);
    printf("Total Faults: %d\n", readFaults + writeFaults);
    // print percentage of read and write faults
    printf("Write Fault Percentage: %.2f%%\n", (float) writeFaults / (readFaults + writeFaults) * 100);
}

int main(int argc, char *argv[]) {

    // check for correct number of arguments
    if (argc != 2) {
        printf("Usage: %s <algorithm>\n", argv[0]);
        exit(1);
    }

    // check for valid algorithm
    if (strcmp(argv[1], "LRU") == 0) {
        algorithm = "LRU";
    } else if (strcmp(argv[1], "LFU") == 0) {
        algorithm = "LFU";
    } else if (strcmp(argv[1], "Random") == 0) {
        algorithm = "Random";
    } else {
        printf("Invalid algorithm\n");
        printf("Usage: %s <Random/LRU/LFU>\n", argv[0]);
        exit(1);
    }

    pthread_t thread;

    // start animation thread
    pthread_create(&thread, NULL, runAnimation, NULL);

    // initialize the inverted page table
    for (int i = 0; i < MAX_FRAMES; i++) {
        table[i].pid = -1;
        table[i].page = -1;
        table[i].reference = 0;
        table[i].frequency = 1;
        table[i].dirty = 0;
        table[i].valid = 0;
    }

    // run simulation
	Simulate(1000 * MAX_FRAMES);

	// stop animation thread
    stopAnimation();

    // join animation thread
    pthread_join(thread, NULL);

    // print results
	printResults();

    return 0;
}
