//
// Created by GYM on 2025/9/28.
//

#include "Malloc.h"
#include "stdio.h"

typedef struct memy_node *PNode; // 定义节点指针
typedef struct memy_node
{
    uint32_t offset; // 与内存首地址的偏移
    uint32_t size;   // 本节点带的内存大小
    PNode prior;     // 前驱指针域
    PNode next;      // 后继指针域
} MemoryNode;

/* 编译器识别 */
#if defined(__IAR_SYSTEMS_ICC__) || defined(__ICCARM__)
    /* 当前是 IAR 编译器 */
    #define COMPILER_IAR
#elif defined(__CC_ARM)
    /* 当前是 Keil Arm Compiler 5 (AC5) */
    #define COMPILER_KEIL_AC5
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6000000)
    /* 当前是 Keil Arm Compiler 6 (AC6) */
    #define COMPILER_KEIL_AC6
#else
    #define COMPILER_UNKNOW
#endif


/* 通过不同编译器的语法来指定内存池地址 */
#if defined(COMPILER_KEIL_AC5)
			#define MEMORY_POOL_INOF(serial, size, name, address) __align(4) uint8_t mem_##name[size]__attribute__((at(address)));
					MEMORY_POOL_TABLE
			#undef MEMORY_POOL_INOF
#elif defined(COMPILER_KEIL_AC6)
			    #define MEMORY_POOL_INOF(serial, size, name, address) \
        __attribute__((aligned(4))) \
        __attribute__((section(".ARM.__at_" #address))) \
        uint8_t mem_##name[size];
					MEMORY_POOL_TABLE
			#undef MEMORY_POOL_INOF
#elif defined(COMPILER_IAR)
#define MEMORY_POOL_INOF(serial, size, name, address) __no_init uint8_t mem_##name[size] @ address;
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
#elif defined(COMPILER_UNKNOW)
#define MEMORY_POOL_INOF(serial, size, name, address) uint8_t mem_##name[size];
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
#endif

// 头 尾节点
MemoryNode HedaNode[MEMORY_POOL_MAX], EndNode[MEMORY_POOL_MAX]; 

// 内存大小表
uint32_t memory_size_table[MEMORY_POOL_MAX] = {
#define MEMORY_POOL_INOF(serial, size, name, address) size,
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
}; 

// 内存基地址表
uint8_t *memory_base_table[MEMORY_POOL_MAX] = {
#define MEMORY_POOL_INOF(serial, size, name, address) mem_##name,
           MEMORY_POOL_TABLE
#undef MEMORY_POOL_INOF
}; 

/**
 * @brief 格式化内存
 * @param dest 内存首地址
 * @param value 格式化值
 * @param size 长度
 */
void mymemset(void *dest, uint8_t value, uint32_t size)
{
    uint8_t *addr = dest;
    while (size--)
    {
        *addr++ = value;
    }
}

/**
 * @brief 内存复制
 * @param dest 目标地址
 * @param src 源地址
 * @param size 长度
 */
void mymemcpy(void *dest, void *src, uint32_t size)
{
    uint8_t *addr = dest;
    uint8_t *addr1 = src;
    while (size--)
    {
        *addr++ = *addr1++;
    }
}

/**
 * @brief 初始化内存
 */
void mem_init() // 初始化内存链表
{
    for (int i = 0; i < MEMORY_POOL_MAX; ++i)
    {
        HedaNode[i].offset = 0;
        HedaNode[i].size = 0;
        HedaNode[i].prior = NULL;
        HedaNode[i].next = &EndNode[i];

        EndNode[i].offset = memory_size_table[i];
        EndNode[i].size = 0;
        EndNode[i].prior = &HedaNode[i];
        EndNode[i].next = NULL;

        mymemset(memory_base_table[i], 0, memory_size_table[i]);
    }
}


#ifndef MEMORY_FRAGMENT_OPT // 内存碎片优化
/**
 * @brief 内存申请
 * @note  4字节对齐
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *mymalloc(MemPool_e mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POOL_MAX)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemoryNode); // 实际要申请的内存大小 包含节点空间
    if (relsize % 4)                    // 4字节对齐
        relsize += (4 - (relsize % 4)); // 不满4字节就补齐
    p = &HedaNode[mem_num];                      // 导入头节点
    while (p->next)                     // 下一节点存在
    {
        if ((p->next->offset - (p->offset + p->size)) > relsize) // 本节点和下节点间剩余的内存>要申请的内存大小
        {
            menaddr = memory_base_table[mem_num] + (p->offset) + (p->size);
            pnew = (PNode)menaddr; // 新节点的地址=内存池基地址+本节点偏移量+本节点内存大小
            pnew->prior = p;
            pnew->next = p->next;                  // 新节点的Pnext=本节点Pnext
            pnew->size = relsize;                  // 新节点的内存大小
            pnew->offset = p->offset + p->size;    // 新节点偏移量
            p->next = pnew;                        // 本节点Pnext=新节点的地址
            mymemset(pnew + 1, 0, size);           // 初始化内存
            return (void *)(((PNode)menaddr) + 1); // 返回去除节点信息的地址
        }
        else
            p = p->next; // 导入下一个节点
    }
    return NULL; // 申请失败
}
#else
/**
 * @brief 内存申请
 * @note  4字节对齐 最大化利用碎片内存
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *mymalloc(MemPool_e mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POOL_MAX)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemoryNode); // 实际要申请的内存大小 包含节点空间
    if (relsize % 4)                    // 4字节对齐 不满4字节就补齐
    {
        relsize += (4 - (relsize % 4));
    }
    p = &HedaNode[mem_num];             // 导入头节点
    uint8_t node_deep = 0;              // 节点深度
    uint8_t valid_noed_num = 0;         // 有效节点数量
    uint32_t min_szie = 0xFFFFFFFF;     // 最小有效节点大小
    uint8_t min_size_deep = 0;          // 最小有效节点深度
    while (p->next)                     //寻找最佳大小的节点
    {
        uint32_t free_size = p->next->offset - (p->offset + p->size);
        if (free_size >= relsize) // 本节点和下节点间剩余的内存>=要申请的内存大小
        {
            if (free_size < min_szie)
            {
                min_szie = free_size;
                min_size_deep = node_deep;
            }
            if (++valid_noed_num >= FIND_MAX_DEEP)
                break;
        }
        p = p->next;
        node_deep++;
    }
    if (valid_noed_num == 0)    // 没有找到有效节点
    {
        return NULL;
    }

    p = &HedaNode[mem_num];             // 重新导入头节点
    for (int i = 0; i < min_size_deep; ++i)
    {
        p = p->next;
    }
    menaddr = memory_base_table[mem_num] + (p->offset) + (p->size);
    pnew = (PNode)menaddr; // 新节点的地址=内存池基地址+本节点偏移量+本节点内存大小
    pnew->prior = p;
    pnew->next = p->next;                  // 新节点的Pnext=本节点Pnext
    pnew->size = relsize;                  // 新节点的内存大小
    pnew->offset = p->offset + p->size;    // 新节点偏移量
    p->next = pnew;                        // 本节点Pnext=新节点的地址
    mymemset(pnew + 1, 0, size);           // 初始化内存
    return (void *)(((PNode)menaddr) + 1); // 返回去除节点信息的地址
}
#endif

/**
 * @brief 重新申请内存
 * @param mem_num 内存池编号 @ref MemPool_e
 * @param ptr 内存首地址
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *myrealloc(MemPool_e mem_num, void *ptr, uint32_t size)
{
    if (ptr == NULL)
        return mymalloc(mem_num, size);
    if (size == 0)
    {
        myfree(ptr);
        return NULL;
    }
    PNode p = ((PNode)ptr) - 1;
    if (p->size >= size)
    {
        return ptr;
    }
    uint32_t relsize = size + sizeof(MemoryNode);
    uint32_t behind_size = p->next->offset - (p->offset + p->size); // 本节点和下节点间剩余的 优先寻找原地址下是否有充足空间
    if (relsize % 4)
    {
        relsize += (4 - (relsize % 4));
    }
    if (behind_size + p->size >= relsize)
    {
        p->size = relsize;
        return ptr;
    }

    uint8_t *menaddr = (uint8_t *)mymalloc(mem_num, size);  // 原地址下空间不足， 申请新的内存
    if (menaddr == NULL)
    {
        printf("malloc error\n");
        return NULL;
    }
    mymemcpy(menaddr, ptr, p->size);    // 内存拷贝
    myfree(ptr);
    return (void *)menaddr;
}


/**
 * @brief 释放内存
 * @param ptr 内存首地址
 */
void myfree(void *ptr)
{
    PNode p = NULL;
    if (ptr == NULL)
        return;
    p = ((PNode)ptr) - 1;      // 点位到内存地址前的节点信息
    if (p->prior && p->next) // 防止二次释放产生错误
    {
        p->prior->next = p->next;  // 本上节点的上一节点的Pnext=本节点的下一节点地址
        p->next->prior = p->prior; // 本上节点的下一节点的Prior=本节点的上一节点地址
        p->prior = NULL;
        p->next = NULL;
    }
}

/**
 * @brief 打印内存使用情况
 * @param mem_num 内存池编号 @ref MemPool_e
 */
void mem_print(MemPool_e mem_num)
{
    uint32_t use_size = 0;
    PNode p = &HedaNode[mem_num];
    while (p->next)
    {
        use_size += p->size + sizeof(MemoryNode);
        p = p->next;
    }
    printf("pool_num:%d Used %dKB/%dKB:%.2f%%\n", mem_num, use_size / 1024, memory_size_table[mem_num] / 1024, (float)use_size * 100.0f / (float)memory_size_table[mem_num]);
}
