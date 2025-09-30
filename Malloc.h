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

/**
 * @brief 内存池数量
 */
#define MEMORY_POLL_NUM 3

/**
 * @brief 内存池的大小(单位: 字节)
 */
#define MEM0_MAX_SIZE (1024 * 32)
#define MEM1_MAX_SIZE (1024 * 24)
#define MEM2_MAX_SIZE (1024 * 5)
#define MEM3_MAX_SIZE (1024 * 50)

/**
 * @brief 内存池编号枚举，用于内存申请的传参
 */
typedef enum
{
    MEM_NUM0 = 0,
    MEM_NUM1 = 1,
    MEM_NUM2 = 2,
}MemPoolNum_e;


void mem_init();
void *mymalloc(MemPoolNum_e mem_num, uint32_t size);
void myfree(void *ptr);
void mymemset(void *dest,uint8_t value,uint32_t size);
void mymemcpy(void *dest, void *src, uint32_t size);
void mem_print(MemPoolNum_e mem_num);
void *myrealloc(MemPoolNum_e mem_num, void *ptr, uint32_t size);



#endif //MALLOC_MALLOC_H
