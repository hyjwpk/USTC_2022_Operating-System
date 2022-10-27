/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
static char *mem_start_brk;  /* points to first byte of heap */
static char *mem_brk;        /* points to last byte of heap */
static char *mem_max_addr;   /* largest legal heap address */ 

/* 
 * mem_init - initialize the memory system model
 */
void mem_init(void)
{
    /*
        TODO: 
        调用 sbrk, 初始化 mem_start_brk、mem_brk、以及 mem_max_addr
        此处增长堆空间大小为 MAX_HEAP
    */
    
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk;
    
    /*
        TODO:
            模拟堆增长
            incr: 申请 mem_brk 的增长量
            返回值: 旧 mem_brk 值
        HINTS:
        1. 若 mem_brk + incr 没有超过实际的 mem_max_addr 值，直接推进 mem_brk 值即可
        2. 若 mem_brk + incr 超过实际的 mem_max_addr 值，需要调用 sbrk 为内存分配器掌管的内存扩容
        3. 每次调用 sbrk 时， mem_max_addr 增量以 MAXHEAP对齐
    */
    
    

    
    return (void *)old_brk;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
size_t mem_pagesize()
{
   /*return (size_t)getpagesize();*/
   return 4096;
}
