#include <stdio.h>
#include <stdlib.h>
#include "PageSim.h"

void Simulate(int rounds){
    for (int i = 0; i < rounds; i++) {
        int add = rand() % 2048;
        add = add << 12;
        int w = rand() % 2;
        int p = rand() % 70;
        Access(p, add, w);
    }

    for (int i = 0; i < 10; i++) {
        int e = rand() % 70;
        Terminate(e);
    }

    for (int i = 0; i < rounds; i++) {
        int add = rand() % 2048;
        add = add << 12;
        int w = rand() % 2;
        int p = rand() % 70;
        Access(p, add, w);
    }
}