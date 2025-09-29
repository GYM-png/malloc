//
// Created by GYM on 2025/9/28.
//

#ifndef MALLOC_MALLOC_H
#define MALLOC_MALLOC_H

#include <stdint.h>

// 内存碎片优化 0:不优化 1:优化
// 申请内存时将寻找多个满足大小的节点位置，并分配最小的节点，寻找深度为 FIND_MAX_DEEP
#define MEMORY_FRAGMENT_OPT

#ifdef MEMORY_FRAGMENT_OPT
#define FIND_MAX_DEEP 5 // 寻找深度 最多寻找5个节点
#endif

#define MEMORY_POLL_NUM 3

#define MEM0_MAX_SIZE (1024 * 32)
#define MEM1_MAX_SIZE (1024 * 24)
#define MEM2_MAX_SIZE (1024 * 5)

#define MEM_NUM0 0
#define MEM_NUM1 1
#define MEM_NUM2 2

void mem_init();
void *mymalloc(uint8_t mem_num, uint32_t size);
void myfree(void *ptr);
void mymemset(void *dest,uint8_t value,uint32_t size);
void mymemcpy(void *dest, void *src, uint32_t size);
void mem_print(uint8_t mem_num);
void *myrealloc(uint8_t mem_num, void *ptr, uint32_t size);



#endif //MALLOC_MALLOC_H
