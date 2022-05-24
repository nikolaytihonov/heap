#include <stdio.h>
#include <stdint.h>

#define cu_round2_up(val,bit) (((val>>bit) + !!(val&((1<<bit)-1)) ) << bit)

typedef struct {
    void* start;
    unsigned int size;
} mheap_t;

typedef struct {
    void* prev;
    unsigned int size;
} mblock_t;

#define MBLOCK_ALLOCATED    (1<<0)
#define MBLOCK_SIZE(size)   ((size) & ~MBLOCK_ALLOCATED)

void heap_init(mheap_t* heap, void* start, unsigned int size)
{
    heap->start = start;
    heap->size = size;

    mblock_t* block = (mblock_t*)heap->start;
    block->prev = NULL;
    block->size = size;
}

void heap_split(mblock_t* block, unsigned int size)
{
    mblock_t* split = (mblock_t*)((uint8_t*)block + size);
    split->prev = block;
    split->size = block->size - size;
    block->size = size;
}

int main()
{
    printf("mblock_t\t%u\n", sizeof(mblock_t));
    return 0;
}
