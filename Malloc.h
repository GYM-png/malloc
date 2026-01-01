/**
 * @file Malloc.h
 * @author GYM (480609450@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */
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

/**
 * @brief 自定义内存池地址，大小，枚举名
 * @brief Custom memory pool address, size, enumeration name
 * @note 可增加或删减，但是序号必须从0开始递增
 * @note May be added or removed, but the index must start from 0 and increase sequentially
 * 
 */
                /* 序号, 内存池大小, 内存池名字, 内存池起始地址 */
                /* Index, Memory pool size, Memory pool name, Memory pool start address */
#define MEMORY_POOL_TABLE   \
    MEMORY_POOL_INOF(0, 1024 * 64, MEMORY_CCM, 0x10000000)  \
    MEMORY_POOL_INOF(1, 1024 * 16, MEMORY_SRAM_2, 0x2001C000)  \
    MEMORY_POOL_INOF(2, 1024 * 64, MEMORY_SRAM_3, 0x20020000)  

typedef enum
{
    #define MEMORY_POOL_INOF(serial, size, name, address) name = serial,
    MEMORY_POOL_TABLE
    #undef MEMORY_POOL_INOF
}MemPool_e;

void lw_malloc_init();
void *lw_malloc(MemPool_e mem_num, uint32_t size);
void lw_free(void *ptr);
void *lw_malloc_auto(uint32_t size);
void *lw_realloc(MemPool_e mem_num, void *ptr, uint32_t size);

void lw_memset(void *dest,uint8_t value,uint32_t size);
void lw_memcpy(void *dest, void *src, uint32_t size);

float lw_get_memory_rate(MemPool_e mem_num);
void lw_memory_list(uint8_t *buffer, uint16_t buffer_size);

#endif //MALLOC_MALLOC_H
