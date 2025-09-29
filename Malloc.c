//
// Created by GYM on 2025/9/28.
//

#include "Malloc.h"
#include "stdio.h"

#if MEMORY_POLL_NUM < 1
#error "MEMORY_POLL_NUM must be greater than 0"
#endif

typedef struct memy_node *PNode; // 定义节点指针
typedef struct memy_node
{
    uint32_t offset; // 与内存首地址的偏移
    uint32_t size;   // 本节点带的内存大小
    PNode prior;     // 前驱指针域
    PNode next;      // 后继指针域
} MemeyNode;


#if MEMORY_POLL_NUM == 1
uint8_t membase0[MEM0_MAX_SIZE];                 // SRAM内存池 4字节对齐
MemeyNode HedaNode[1], EndNode[1];              // 头尾节点
uint32_t mem_size_table[1] = {MEM0_MAX_SIZE};   // 内存size表
uint8_t *membase_table[1] = {membase0};          // 内存基地址表
#elif MEMORY_POLL_NUM == 2
uint8_t membase0[MEM0_MAX_SIZE];                              // SRAM内存池 4字节对齐
uint8_t membase1[MEM1_MAX_SIZE];                             // SRAM内存池 4字节对齐
MemeyNode HedaNode[2], EndNode[2];                           // 头尾节点
uint32_t mem_size_table[2] = {MEM0_MAX_SIZE, MEM1_MAX_SIZE}; // 内存size表
uint8_t *membase_table[2] = {membase0, membase1};             // 内存基地址表
#elif MEMORY_POLL_NUM == 3
uint8_t membase0[MEM0_MAX_SIZE];                                                         // SRAM内存池 4字节对齐
uint8_t membase1[MEM1_MAX_SIZE];                                                        // SRAM内存池 4字节对齐
uint8_t membase2[MEM2_MAX_SIZE];                                                        // SRAM内存池 4字节对齐
MemeyNode HedaNode[3], EndNode[3];                                                      // 头尾节点
uint32_t mem_size_table[3] = {MEM0_MAX_SIZE, MEM1_MAX_SIZE, MEM2_MAX_SIZE}; // 内存size表
uint8_t *membase_table[3] = {membase0, membase1, membase2};                  // 内存基地址表
#endif

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
    for (int i = 0; i < MEMORY_POLL_NUM; ++i)
    {
        HedaNode[i].offset = 0;
        HedaNode[i].size = 0;
        HedaNode[i].prior = NULL;
        HedaNode[i].next = &EndNode[i];

        EndNode[i].offset = mem_size_table[i];
        EndNode[i].size = 0;
        EndNode[i].prior = &HedaNode[i];
        EndNode[i].next = NULL;

        mymemset(membase_table[i], 0, mem_size_table[i]);
    }
}


#ifndef MEMORY_FRAGMENT_OPT // 内存碎片优化
/**
 * @brief 内存申请
 * @note  4字节对齐
 * @param mem_num 内存池编号
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *mymalloc(uint8_t mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POLL_NUM)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemeyNode); // 实际要申请的内存大小 包含节点空间
    if (relsize % 4)                    // 4字节对齐
        relsize += (4 - (relsize % 4)); // 不满4字节就补齐
    p = &HedaNode[mem_num];                      // 导入头节点
    while (p->next)                     // 下一节点存在
    {
        if ((p->next->offset - (p->offset + p->size)) > relsize) // 本节点和下节点间剩余的内存>要申请的内存大小
        {
            menaddr = membase_table[mem_num] + (p->offset) + (p->size);
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
 * @param mem_num 内存池编号
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *mymalloc(uint8_t mem_num, uint32_t size)
{
    if (mem_num >= MEMORY_POLL_NUM)
    {
        printf("内存池编号错误\n");
        return NULL;
    }
    PNode p = NULL;
    PNode pnew = NULL;
    uint32_t relsize;
    uint8_t *menaddr;
    relsize = size + sizeof(MemeyNode); // 实际要申请的内存大小 包含节点空间
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
    menaddr = membase_table[mem_num] + (p->offset) + (p->size);
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
 * @param mem_num 内存池编号
 * @param ptr 内存首地址
 * @param size 申请内存大小(单位:字节)
 * @return 申请到的内存首地址
 */
void *myrealloc(uint8_t mem_num, void *ptr, uint32_t size)
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
    uint32_t relsize = size + sizeof(MemeyNode);
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
 * @param mem_num 内存池编号
 */
void mem_print(uint8_t mem_num)
{
    uint32_t use_size = 0;
    PNode p = &HedaNode[mem_num];
    while (p->next)
    {
        use_size += p->size + sizeof(MemeyNode);
        p = p->next;
    }
    printf("pool_num:%d Used %dKB/%dKB:%.2f%%\n", mem_num, use_size / 1024, mem_size_table[mem_num] / 1024, (float)use_size * 100.0f / (float)mem_size_table[mem_num]);
}
