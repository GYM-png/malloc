#include <stdio.h>
#include <stdint.h>
#include "lw_malloc.h"


/**
 * @brief 检查内存数据
 * @param addr 地址
 * @param value 目标值
 * @param size 长度
 */
void mem_ceck(const uint8_t *addr, const uint8_t value, uint32_t size)
{
    if (addr == NULL)
    {
        printf("addr is null\n");
        return;
    }
    for (int i = 0; i < size; ++i) {
        if (addr[i] != value)
        {
            printf("%p check error at %d\n", addr, i);
            return;
        }
    }
    printf("check success\n");
}


void malloc_test1()
{
#define DATA1_VALUE 0xAA
#define DATA2_VALUE 0xBB
#define DATA3_VALUE 0xCC
#define DATA4_VALUE 0xDD
#define DATA5_VALUE 0xEE
#define DATA6_VALUE 0xFF

#define DATA1_SIZE (1024 * 1)
#define DATA2_SIZE (1024 * 3)
#define DATA3_SIZE (1024 * 5)
#define DATA4_SIZE (1024 * 2)
#define DATA5_SIZE (1024 * 1)
#define DATA6_SIZE (1024 * 2)
    uint8_t *data1 = NULL, *data2 = NULL, *data3 = NULL, *data4 = NULL, *data5 = NULL, *data6 = NULL;
    data1 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA1_SIZE);
    data2 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA2_SIZE);
    data3 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA3_SIZE);
    data4 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA4_SIZE);
    data5 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA5_SIZE);
    if(data1 == NULL || data2 == NULL || data3 == NULL || data4 == NULL || data5 == NULL)
    {
        printf("malloc error\n");
        if (data1)
            lw_free(data1);
        if (data2)
            lw_free(data2);
        if (data3)
            lw_free(data3);
        if (data4)
            lw_free(data4);
        if (data5)
            lw_free(data5);
        return;
    }
    lw_memset(data1, DATA1_VALUE, DATA1_SIZE);
    lw_memset(data2, DATA2_VALUE, DATA2_SIZE);
    lw_memset(data3, DATA3_VALUE, DATA3_SIZE);
    lw_memset(data4, DATA4_VALUE, DATA4_SIZE);
    lw_memset(data5, DATA5_VALUE, DATA5_SIZE);

    mem_ceck(data1, DATA1_VALUE, DATA1_SIZE);
    mem_ceck(data2, DATA2_VALUE, DATA2_SIZE);
    mem_ceck(data3, DATA3_VALUE, DATA3_SIZE);
    mem_ceck(data4, DATA4_VALUE, DATA4_SIZE);
    mem_ceck(data5, DATA5_VALUE, DATA5_SIZE);
    // 此时的内存分配情况
    // data1 | data2 | data3 | data4 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |
    lw_free(data2); data2 = NULL;
    lw_free(data4); data4 = NULL;
    // 此时的内存分配情况
    // data1 |       | data3 |       | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |

    data6 = (uint8_t *)lw_malloc(MEMORY_CCM, DATA6_SIZE);//DATA6_SIZE 为2K， 开启内存碎片优化后，会申请到原data4(2K)的位置，否则会申请到原data2(3K)的位置
    if (data6 == NULL)
    {
        printf("error 内存申请失败\n");
        return;
    }
    #ifdef MEMORY_FRAGMENT_OPT
    if (data3 < data6 && data6 < data5 )
    {
        printf("success 内存碎片优化测试成功\n");
    }
    else
    {
        printf("error 内存碎片优化测试失败");
    }
    #endif
    // 此时的内存分配情况 (开启内存碎片优化的情况)
    // data1 |       | data3 | data6 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 1K    |
    uint8_t *before_ptr5 = data5;
    lw_memset(data6, DATA6_VALUE, DATA6_SIZE);
    printf("brefore data 5 addr %p\n", data5);
    data5 = (uint8_t *)lw_realloc(MEMORY_CCM, data5, 1024 * 2);
    printf("after   data 5 addr %p\n", data5);
    if (data5 == NULL)
    {
        printf("error 内存申请失败\n");
    }
    else
    {
        lw_memset(data5, DATA5_VALUE, 1024 * 2);
    }
    if (before_ptr5 == data5)
    {
        printf("success realloc 测试1\n");
    }
    else
    {
        printf("error realloc 测试1\n");
    }
    // 此时的内存分配情况
    // data1 |       | data3 | data6 | data5 |
    //| 1K   | 3K    | 5K    | 2K    | 2K    |
    uint8_t *before_ptr1 = data1;
    lw_free(data2);
    printf("brefore data 1 addr %p\n", before_ptr1);
    data1 = (uint8_t *)lw_realloc(MEMORY_CCM, data1, 1024 * 5); //这里申请必须大于1024 * 4 + 24 字节，才能使data1首地址改变
    if (data1 == NULL)
    {
        printf("内存申请失败\n");
    }
    else
    {
        lw_memset(data1, DATA1_VALUE, 1024 * 5);
    }
    printf("after   data 1 addr %p\n", data1);
    if (before_ptr1 < data1)
    {
        printf("success realloc 测试2\n");
    }
    else
    {
        printf("error realloc 测试2\n");
    }
    // 此时的内存分配情况
    //       |       | data3 | data6 | data5 | data1 |
    //| 1K   | 3K    | 5K    | 2K    | 2K    | 5K    |
    mem_ceck(data1, DATA1_VALUE, 1024 * 5);
    mem_ceck(data2, DATA2_VALUE, DATA2_SIZE);
    mem_ceck(data3, DATA3_VALUE, DATA3_SIZE);
    mem_ceck(data4, DATA4_VALUE, DATA4_SIZE);
    mem_ceck(data5, DATA5_VALUE, DATA5_SIZE);

    lw_memory_list(data1, 1024);
    printf("%s\n", data1);
    // lw_free(data1);
    lw_free(data2);
    lw_free(data3);
    lw_free(data4);
    lw_free(data5);
    lw_free(data6);
    lw_memory_list(data1, 1024);
    printf("%s\n", data1);
    lw_free(data1);
}

int main(void) {
    lw_malloc_init();
    malloc_test1();
    return 0;

}
