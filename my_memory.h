#ifndef MY_MEMORY_H
#define MY_MEMORY_H

#include "interface.h"



struct buddy 
{
    int address;
    struct buddy *next;
    struct buddy *prev; 
    int len;
    int capacity;
    bool taken;
    uint64_t bitmap;
};

struct buddy *first;
void *start;
int m_type;

int roundPowerTwo(float size);

struct buddy *newBuddy(int address, struct buddy *next, struct buddy *prev, int len);

void *buddy_malloc(int size);
void *slab_malloc(int size);

void buddy_free(void *ptr);
void slab_free(void *ptr);



#endif