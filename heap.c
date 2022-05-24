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
#define MBLOCK_ALIGN        3

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

void heap_join(mblock_t* block, mblock_t* other)
{
    if (block < other)
    {
        mblock_t* next = (mblock_t*)((uint8_t*)other + other->size);
        next->prev = block;
        block->size += other->size;
    }
    else if (block > other)
    {
        mblock_t* next = (mblock_t*)((uint8_t*)block + block->size);
        next->prev = other;
        other->size += block->size;
    }
}

mblock_t* heap_alloc(mheap_t* heap, unsigned int _size)
{
    unsigned int size = cu_round2_up(_size + sizeof(mblock_t), MBLOCK_ALIGN);
    mblock_t* block = (mblock_t*)((uint8_t*)heap->start);
    mblock_t* end = (mblock_t*)((uint8_t*)heap->start + heap->size);
    while ((block->size & MBLOCK_ALLOCATED) || MBLOCK_SIZE(block->size) < size)
    {
        block = (mblock_t*)((uint8_t*)block + MBLOCK_SIZE(block->size));
        if (block >= end) return NULL;
    }

    if (block->size > size)
    {
        heap_split(block, size);
    }

    block->size |= MBLOCK_ALLOCATED;
    return block;
}

void heap_free(mblock_t* block)
{
    block->size &= ~MBLOCK_ALLOCATED;
    mblock_t* prev = block->prev;
    mblock_t* next = (mblock_t*)((uint8_t*)block + block->size);
    if (!(prev->size & MBLOCK_ALLOCATED))
        heap_join(block, prev);
    if (!(next->size & MBLOCK_ALLOCATED))
        heap_join(block, next);
}


static void heap_debug(mheap_t* heap)
{
    mblock_t* block = (mblock_t*)((uint8_t*)heap->start);
    mblock_t* end = (mblock_t*)((uint8_t*)heap->start + heap->size);
    printf("heap\t%p\t%p\n", block, end);
    while (block < end && MBLOCK_SIZE(block->size))
    {
        printf("block\t%p\t%u\n", block, block->size);
        block = (mblock_t*)((uint8_t*)block + MBLOCK_SIZE(block->size));
    }
}

static uint8_t internal_heap[4096];

int main()
{
    mheap_t heap;
    heap_init(&heap, internal_heap, sizeof(internal_heap));
    
    printf("heap_alloc\t%p\n", heap_alloc(&heap, 8));
    printf("heap_alloc\t%p\n", heap_alloc(&heap, 8));
    mblock_t* m3 = heap_alloc(&heap, 12);
    printf("heap_alloc\t%p\n", m3);
    printf("heap_alloc\t%p\n", heap_alloc(&heap, 8));
    heap_free(m3);
    printf("heap_alloc\t%p\n", heap_alloc(&heap, 8));

    heap_debug(&heap);
    return 0;
}
