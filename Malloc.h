//
// Created by GYM on 2025/9/28.
//

#ifndef MALLOC_MALLOC_H
#define MALLOC_MALLOC_H

#include <stdint.h>


/**
 * @brief 内存碎片优化 0:不优化 1:优化
 * @note  开启内存碎片优化后，申请内存时会寻找多个满足大小的节点位置，并分配最小的节点，最大寻找深度为 FIND_MAX_DEEP
 */
#define MEMORY_FRAGMENT_OPT

/**
 * @brief 最大寻找深度
 */
#ifdef MEMORY_FRAGMENT_OPT
#define FIND_MAX_DEEP 5
#endif

#define AUTO_SELECT_MEMORY_POOL

                /* 序号, 内存池大小, 内存池名字, 内存池起始地址 */
#define MEMORY_POOL_TABLE   \
    MEMORY_POOL_INOF(0, 1024 * 64, MEMORY_CCM, 0x10000000)  \
    MEMORY_POOL_INOF(1, 1024 * 16, MEMORY_SRAM_2, 0x2001C000)  \
    MEMORY_POOL_INOF(2, 1024 * 64, MEMORY_SRAM_3, 0x20020000)  
    // MEMORY_POOL_INOF(3, 1024 * 50, MEMORY_BASE)  


typedef enum
{
    #define MEMORY_POOL_INOF(serial, size, name, address) name,
    MEMORY_POOL_TABLE
    #undef MEMORY_POOL_INOF
    MEMORY_POOL_MAX
}MemPool_e;

void mem_init();
void *mymalloc(MemPool_e mem_num, uint32_t size);
void myfree(void *ptr);
void mymemset(void *dest,uint8_t value,uint32_t size);
void mymemcpy(void *dest, void *src, uint32_t size);
void mem_print(MemPool_e mem_num);
void *myrealloc(MemPool_e mem_num, void *ptr, uint32_t size);




#endif //MALLOC_MALLOC_H
