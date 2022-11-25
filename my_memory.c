#include "my_memory.h"



// Helper function that rounds up to the next largest power of two
int roundPowerTwo(float size)
{
	int power = 0;
	while (size > 1)
	{
		size /= 2;
		power++;
	}

	size = 1;
	for (int i = 0; i < power; i++)
	{
		size *= 2;
	}

	return size;
}


// Creates a new linked list node; Represents an allocatable region in memory.
struct buddy *newBuddy(int address,struct buddy *next, struct buddy *prev, int len)
{
	struct buddy *new = malloc(sizeof(struct buddy));

	new->address = address;
	new->next = next;
	new->prev = prev;
	new->len = len;     //total length of buddy

	new->taken = false; //in slab, it means full, if partial: its not 'taken'

    new->bitmap = 0;    //used in slab only;
    new->capacity = 0;  //used in slab only, if 0, not used as a slab;

	return new;
}


// Determines if the node is a slab or just a simple buddy node; Not a slab if the block capacity is zero.
bool notaSlab(struct buddy *block)
{
    return (block->capacity == 0);
}


// Used to keep track of capacity within the slab; Slab is fully allocated when 63 
int malloc_bitmap(struct buddy *block)
{
    for (int i=0; i<64; i++)
    {
        if((block->bitmap >> i) % 2 == 0)
        {
            block->bitmap |= ((uint64_t)1 << i);

            if (i == 63)
            {
                block->taken = true;
            }

            return i;
        }
    }
}


// Updates the bit map following a free; if Slab was previously fully allocated, it is now no longer.
void free_bitmap(struct buddy *block,int index)
{
    block->bitmap &= ~((uint64_t)1 << index);
    
    if (block->taken)
    {
        block->taken = false;
    }
}


/*
 *Split creates a new node in the linked list
 *This node is created to the right of the node split is called on
 *Again, this represents a division of the region of memory
 *The size of the memory division of the called buddy (node) is halved
 *New created region is also given this size
 *Procedure is as follows:
 *[fl]
 *[f][l]
 *former [f] is old, latter [l] is new
*/
void split(struct buddy *block)
{
	struct buddy *latter = newBuddy((block->address) + (block->len)/2, (block->next), (block), (block->len)/2);
	block->len /= 2;
	block->next = latter;
}


/*Opposite of the split procedure, this is called during free.
  Combines adjacent, equally sized memory regions to serve future malloc calls */
void merge(struct buddy *block)
{
	struct buddy *temp = block->next;
	block->next = block->next->next;

	if (block->next)
	{
		block->next->prev = block;
	}

	free(temp);
	block->len *= 2;
}

/*
──────────────────────────────────────────────────────────────────────────────────────
─██████████████────██████──██████──████████████────████████████────████████──████████─
─██░░░░░░░░░░██────██░░██──██░░██──██░░░░░░░░████──██░░░░░░░░████──██░░░░██──██░░░░██─
─██░░██████░░██────██░░██──██░░██──██░░████░░░░██──██░░████░░░░██──████░░██──██░░████─
─██░░██──██░░██────██░░██──██░░██──██░░██──██░░██──██░░██──██░░██────██░░░░██░░░░██───
─██░░██████░░████──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██────████░░░░░░████───
─██░░░░░░░░░░░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──────████░░████─────
─██░░████████░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██────────██░░██───────
─██░░██────██░░██──██░░██──██░░██──██░░██──██░░██──██░░██──██░░██────────██░░██───────
─██░░████████░░██──██░░██████░░██──██░░████░░░░██──██░░████░░░░██────────██░░██───────
─██░░░░░░░░░░░░██──██░░░░░░░░░░██──██░░░░░░░░████──██░░░░░░░░████────────██░░██───────
─████████████████──██████████████──████████████────████████████──────────██████───────
──────────────────────────────────────────────────────────────────────────────────────
*/
void *buddy_malloc(int size)
{ 

    // If request is less than the minimum,  allocate minimum.
    // Otherwise round to the next nearest power of 2 
    if (size + HEADER_SIZE <= MIN_MEM_CHUNK_SIZE)
    {
        size = MIN_MEM_CHUNK_SIZE;
    }

    else
    {
        size = roundPowerTwo(size + HEADER_SIZE);
    }

    // Current is used for traversing the buddy linked list, candidate is a potential buddy to be allocated
    struct buddy *current = first;
    struct buddy *candidate = NULL;

 

    // Iterate through linked list
    while(current)
    {

        // Buddy is unallocated and size matches memory request
        if (!(current->taken) && (current->len == size))
        {
            current->taken = true;
            return start + HEADER_SIZE + current->address;
        }

        // Buddy is unallocated but the memory space is larger than the request 
        if (!(current->taken) && (current->len > size))
        {
            if ((!candidate) || (candidate && (current->len < candidate->len)))
            {
                candidate = current;
            }
        }
        current = current->next;
    }
    
    // Cannot allocate 
    if (!candidate)
    {
        return (void *)-1;
    }

    // If the requested allocated size is less than the address sizes, split in half into to equal sized address sizes
    while(size < candidate->len)
    {
        split(candidate);
    }
    
    candidate->taken = true;
    return start + HEADER_SIZE + candidate->address;
}


/*
███████╗██████╗ ███████╗███████╗    ██████╗ ██╗   ██╗██████╗ ██████╗ ██╗   ██╗
██╔════╝██╔══██╗██╔════╝██╔════╝    ██╔══██╗██║   ██║██╔══██╗██╔══██╗╚██╗ ██╔╝
█████╗  ██████╔╝█████╗  █████╗      ██████╔╝██║   ██║██║  ██║██║  ██║ ╚████╔╝ 
██╔══╝  ██╔══██╗██╔══╝  ██╔══╝      ██╔══██╗██║   ██║██║  ██║██║  ██║  ╚██╔╝  
██║     ██║  ██║███████╗███████╗    ██████╔╝╚██████╔╝██████╔╝██████╔╝   ██║   
╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝    ╚═════╝  ╚═════╝ ╚═════╝ ╚═════╝    ╚═╝   
*/

void buddy_free(void *ptr)
{
    // Location represents the starting address to be free'd, current used to traversed list
    int location = ptr - start - HEADER_SIZE;
    struct buddy *current = first;

    // Traverses linked list looking for memory with starting address equal to location
    while(current)
    {
        // Found memory region to free, may need to merge adjacent memory regions.
        if (location == current->address)
        {
            current->taken = false;

            bool mergeable = true;

            // Merge if the adjacent memory region is divisible by twice its length 
            while (mergeable)
            {
                mergeable = false;

                if((current->address) % (2*current->len) == 0)
                { 
                    //check if former, i.e., the left node of the two being merged
                    if (current->next)
                    {     
                        // Adjacent memory regions must match in size to be merged.
                        if (!(current->next->taken) && ((current->next->len) == (current->len)))
                        {
                            merge(current);
                            // Continue to check if we can merge again.
                            mergeable = true;
                        }
                    }
                }
                
                /*else is if its latter, i.e. the right node of the two being merged, 
                  Gauranteed a prev (a node to the left), so no check needed */
                else    
                {
                    if (!(current->prev->taken) && ((current->prev->len) == (current->len)))
                    {
                        current = current->prev;
                        merge(current);
                        mergeable = true;
                    }
                }
            }
            break;
        }

        current = current->next;
    }
}


/*
──────────────────────────────────────────────────────────────────
─██████████████──██████──────────██████████████──██████████████───
─██░░░░░░░░░░██──██░░██──────────██░░░░░░░░░░██──██░░░░░░░░░░██───
─██░░██████████──██░░██──────────██░░██████░░██──██░░██████░░██───
─██░░██──────────██░░██──────────██░░██──██░░██──██░░██──██░░██───
─██░░██████████──██░░██──────────██░░██████░░██──██░░██████░░████─
─██░░░░░░░░░░██──██░░██──────────██░░░░░░░░░░██──██░░░░░░░░░░░░██─
─██████████░░██──██░░██──────────██░░██████░░██──██░░████████░░██─
─────────██░░██──██░░██──────────██░░██──██░░██──██░░██────██░░██─
─██████████░░██──██░░██████████──██░░██──██░░██──██░░████████░░██─
─██░░░░░░░░░░██──██░░░░░░░░░░██──██░░██──██░░██──██░░░░░░░░░░░░██─
─██████████████──██████████████──██████──██████──████████████████─
──────────────────────────────────────────────────────────────────
*/

void *slab_malloc(int size)
{
/*
 *check if there exists a slab with capacity = (size+8);
 *if so, allocate in there.
 *if not, simply buddy allocate
 *attempt to create a slab with len = (64*capacity)
 * if successful, allocate in there
 * else return (void *)-1;
*/
    struct buddy *current = first;
    int index;

    while(current)
    {
        // Found valid spot to allocate, must adjaust bit map for slap
        if((current->capacity == (size + HEADER_SIZE)) && !(current->taken))
        {
            index = malloc_bitmap(current);
            return start + HEADER_SIZE + current->address + HEADER_SIZE + (index*current->capacity);
        }
        current = current->next;
    }

    current = first;
    struct buddy *candidate = NULL;
    int total_size;

    // Applies rounding of the size requested
    if ((N_OBJS_PER_SLAB*(size + HEADER_SIZE) + HEADER_SIZE) <= MIN_MEM_CHUNK_SIZE)
    {
        total_size = MIN_MEM_CHUNK_SIZE;
    }

    else
    {
        total_size = roundPowerTwo(N_OBJS_PER_SLAB*(size + HEADER_SIZE) + HEADER_SIZE);
    }

    // Proceed similiarly as we would for buddy allocation
    while(current)
    {
        // Found a size match to serve allocation
        if ((notaSlab(current)) && (current->len == total_size))
        {
            current->capacity = size + HEADER_SIZE;
            index = malloc_bitmap(current);
            return start + HEADER_SIZE + current->address + HEADER_SIZE + (index*current->capacity);
        }

        // Memory region size is larger than the size requested
        if ((notaSlab(current)) && (current->len > total_size))
        {
            // No candidate selected yet or we found a smaller region to potentially serve the request
            if ((!candidate) || (candidate && (current->len < candidate->len)))
            {
                candidate = current;
            }
        }
        current = current->next;
    }
    
    // Cannot serve the memory request
    if (!candidate)
    {
        return (void *)-1;
    }

    // Need to divide the memory region until the length of the region matches the request
    while(total_size < candidate->len)
    {
        split(candidate);
    }
    
    // Adjusts the bit map following the allocation to the slab.
    candidate->capacity = size + HEADER_SIZE;
    index = malloc_bitmap(candidate);

    return start + HEADER_SIZE + candidate->address + HEADER_SIZE + (index*candidate->capacity);
}
	
/*
███████╗██████╗ ███████╗███████╗    ███████╗██╗      █████╗ ██████╗ 
██╔════╝██╔══██╗██╔════╝██╔════╝    ██╔════╝██║     ██╔══██╗██╔══██╗
█████╗  ██████╔╝█████╗  █████╗      ███████╗██║     ███████║██████╔╝
██╔══╝  ██╔══██╗██╔══╝  ██╔══╝      ╚════██║██║     ██╔══██║██╔══██╗
██║     ██║  ██║███████╗███████╗    ███████║███████╗██║  ██║██████╔╝
╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝    ╚══════╝╚══════╝╚═╝  ╚═╝╚═════╝ 
*/

void slab_free(void *ptr)
{
    int location = ptr - start - HEADER_SIZE;
    struct buddy *current = first;

    // Traverses the list
    while(current)
    {
        // Found the region to the free, must adjust in slab.
        if (location >= current->address)
        {
            location = location - current->address - HEADER_SIZE;
            location /= current->capacity;

            // Adjust slab bitmap for the performed free
            free_bitmap(current, location);
            bool mergeable = false;

            // If the slab has no allocations within it, it may be merged.
            if (current->bitmap == 0)
            {
                mergeable = true;
                current->capacity = 0;
            }

            // Follows same process as buddy free.
            while (mergeable)
            {
                mergeable = false;

                // Checks 
                if((current->address) % (2*current->len) == 0)
                {
                    // Checks if node to the right is not NULL
                    if (current->next)
                    {
                        // Adjacent and unallocated, so mergeable
                        if (notaSlab(current->next) && ((current->next->len) == (current->len)))
                        {
                            merge(current);
                            mergeable = true;
                        }
                    }
                }
                
                // Current region is to the right of the two to be merged.
                else
                {
                    // Adjacent and unallocated, so mergeable
                    if (notaSlab(current->prev) && ((current->prev->len) == (current->len)))
                    {
                        current = current->prev;
                        merge(current);
                        mergeable = true;
                    }
                }
            }
            break;
        }
    }
}