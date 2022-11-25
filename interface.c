#include "interface.h"
#include "my_memory.h"



void my_setup(enum malloc_type type, int mem_size, void *start_of_memory)
{
    m_type = type;
    start = start_of_memory;
    first = newBuddy(0, NULL, NULL, MEMORY_SIZE);
}


// Implementation of Buddy and Slab Allocation schemes are found in my_memory.c
void *my_malloc(int size)
{
    if (m_type == MALLOC_BUDDY)
    {
        return buddy_malloc(size);

    } 
    
    else if (m_type == MALLOC_SLAB)
    {
        return slab_malloc(size);
    } 

    else 
    {
        return (void *)-1;
    }
}


// Implementation of Buddy and Slab freeing schemes are found in my_memory.c
void my_free(void *ptr)
{
    // Buddy Allocation Scheme
    if (m_type == MALLOC_BUDDY)
    {
        return buddy_free(ptr);
    } 

    // Slab Allocation Scheme
    else if (m_type == MALLOC_SLAB) 
    {
        return slab_free(ptr);
    }
}
