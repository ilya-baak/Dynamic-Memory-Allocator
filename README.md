# Dynamic-Memory-Allocator
Simulation implementing both buddy and slab memory management schemes. The actual implementation of these algorithms can be found in my_memory.c, where we have
implemented functions for both allocating (malloc) and deallocating (free) memory based on the requested input test file. 

## my_memory.c

This file provides the implementation of the memory allocation simulator using a buddy system as well as slab allocation. 

The buddy system is a memory management algorithm that divides the available memory into blocks of fixed sizes and maintains a linked list of these blocks. The blocks are split or merged dynamically based on the memory allocation and deallocation requests. The splitting and merging is based on the power of 2--such that a block will be divided into two blocks of equal size until the smallest possible memory size big enough to service the request is found. 

In slab allocation, a large block of memory, known as a "slab", is divided into smaller chunks of predetermined size. These chunks are maintained in a linked list, ready for allocation to service memory requests. When a request is made to allocate memory, it can acquire this memory from the slab itself. 

#### Functions
int roundPowerTwo(float size)
A helper function that takes a float size as input and rounds it up to the next largest power of two. It returns an int representing the rounded up size.

struct buddy *newBuddy(int address,struct buddy *next, struct buddy *prev, int len)
Creates a new linked list node, representing an allocatable region in memory. It takes an int address, pointers to the next and previous buddies in the linked list, and an int len representing the total length of the buddy. It returns a pointer to the newly created buddy.

bool notaSlab(struct buddy *block)
Determines if the node is a slab or just a simple buddy node. It takes a pointer to a buddy block as input and returns a boolean value true if the block is not a slab (i.e., the block capacity is not zero), and false otherwise.

int malloc_bitmap(struct buddy *block)
Used to keep track of capacity within the slab. It takes a pointer to a buddy block as input and updates its bitmap to mark the allocation of a block within the slab. It returns an int representing the index of the allocated block within the slab.

void free_bitmap(struct buddy *block,int index)
Updates the bitmap following a free operation. It takes a pointer to a buddy block and an int index representing the index of the freed block within the slab. It clears the corresponding bit in the bitmap to mark the deallocation of the block.

void split(struct buddy *block)
Splits a buddy block into two smaller buddy blocks. It takes a pointer to a buddy block as input and creates a new buddy block to the right of the input block. It updates the length and next/prev pointers of the input block and the newly created block to reflect the split.

void merge(struct buddy *block)
Merges adjacent, equally sized buddy blocks. It takes a pointer to a buddy block as input and merges it with the next buddy block if they are both unallocated and have the same length. It updates the next pointer of the input block and frees the memory of the merged block.

void *buddy_malloc(int size)
Allocates memory using the buddy system. It takes an int size as input, which represents the amount of memory to be allocated. It rounds the size to the nearest power of two, and then searches the buddy linked list for a free block of that size. If a free block is found, it marks it as taken and returns a pointer to the allocated memory. If a free block is not found, it returns a pointer to -1, indicating that the allocation failed.

void buddy_free(void *ptr)
Frees memory that was previously allocated using buddy_malloc(). It takes a pointer to the memory to be freed as input and marks the corresponding buddy block as unallocated. It may also merge adjacent buddy blocks if they are both unallocated and have the same length.



