//
// Created by 抑~风 on 2023-04-17.
//

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#define STACK_SIZE 1024*1024

// 定义CPU上下文结构体
typedef struct context
{
    unsigned long rax;
    unsigned long rbx;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long rbp;
    unsigned long rsp;
    unsigned long r8;
    unsigned long r9;
    unsigned long r10;
    unsigned long r11;
    unsigned long r12;
    unsigned long r13;
    unsigned long r14;
    unsigned long r15;
} context;

// 定义协程结构体
typedef struct coroutine
{
    int cid;  // 协程ID
    void (*func)(void);  // 协程函数指针
    char *stack;  // 协程栈
    context ctx;  // 协程上下文信息
} coroutine;

static coroutine *current;  // 当前协程指针
static coroutine coroutines[2];  // 协程数组

void get_context(coroutine *prev){

    // 保存当前协程的上下文
    asm("mov %%rsp, %0\n" : "=m"(prev->ctx.rsp));
    asm("mov %%rbp, %0\n" : "=m"(prev->ctx.rbp));
    asm("mov %%rax, %0\n" : "=m"(prev->ctx.rax));
    asm("mov %%rbx, %0\n" : "=m"(prev->ctx.rbx));
    asm("mov %%rcx, %0\n" : "=m"(prev->ctx.rcx));
    asm("mov %%rdx, %0\n" : "=m"(prev->ctx.rdx));
    asm("mov %%rsi, %0\n" : "=m"(prev->ctx.rsi));
    asm("mov %%rdi, %0\n" : "=m"(prev->ctx.rdi));
    asm("mov %%r8, %0\n" : "=m"(prev->ctx.r8));
    asm("mov %%r9, %0\n" : "=m"(prev->ctx.r9));
    asm("mov %%r10, %0\n" : "=m"(prev->ctx.r10));
    asm("mov %%r11, %0\n" : "=m"(prev->ctx.r11));
    asm("mov %%r12, %0\n" : "=m"(prev->ctx.r12));
    asm("mov %%r13, %0\n" : "=m"(prev->ctx.r13));
    asm("mov %%r14, %0\n" : "=m"(prev->ctx.r14));
    asm("mov %%r15, %0\n" : "=m"(prev->ctx.r15));

}

// 切换协程到下一个
void switch_context(coroutine *prev, coroutine *next)
{
// 保存当前协程的上下文
    asm("mov %%rsp, %0\n" : "=m"(prev->ctx.rsp));
    asm("mov %%rbp, %0\n" : "=m"(prev->ctx.rbp));
    asm("mov %%rax, %0\n" : "=m"(prev->ctx.rax));
    asm("mov %%rbx, %0\n" : "=m"(prev->ctx.rbx));
    asm("mov %%rcx, %0\n" : "=m"(prev->ctx.rcx));
    asm("mov %%rdx, %0\n" : "=m"(prev->ctx.rdx));
    asm("mov %%rsi, %0\n" : "=m"(prev->ctx.rsi));
    asm("mov %%rdi, %0\n" : "=m"(prev->ctx.rdi));
    asm("mov %%r8, %0\n" : "=m"(prev->ctx.r8));
    asm("mov %%r9, %0\n" : "=m"(prev->ctx.r9));
    asm("mov %%r10, %0\n" : "=m"(prev->ctx.r10));
    asm("mov %%r11, %0\n" : "=m"(prev->ctx.r11));
    asm("mov %%r12, %0\n" : "=m"(prev->ctx.r12));
    asm("mov %%r13, %0\n" : "=m"(prev->ctx.r13));
    asm("mov %%r14, %0\n" : "=m"(prev->ctx.r14));
    asm("mov %%r15, %0\n" : "=m"(prev->ctx.r15));

    // 切换到新协程的上下文
    asm("mov %0, %%rax\n" :: "m"(next->ctx.rax));
    asm("mov %0, %%rbx\n" :: "m"(next->ctx.rbx));
    asm("mov %0, %%rcx\n" :: "m"(next->ctx.rcx));
    asm("mov %0, %%rdx\n" :: "m"(next->ctx.rdx));
    asm("mov %0, %%rsi\n" :: "m"(next->ctx.rsi));
    asm("mov %0, %%rdi\n" :: "m"(next->ctx.rdi));
    asm("mov %0, %%rbp\n" :: "m"(next->ctx.rbp));
    asm("mov %0, %%rsp\n" :: "m"(next->ctx.rsp));
    asm("mov %0, %%r8\n" :: "m"(next->ctx.r8));
    asm("mov %0, %%r9\n" :: "m"(next->ctx.r9));
    asm("mov %0, %%r10\n" :: "m"(next->ctx.r10));
    asm("mov %0, %%r11\n" :: "m"(next->ctx.r11));
    asm("mov %0, %%r12\n" :: "m"(next->ctx.r12));
    asm("mov %0, %%r13\n" :: "m"(next->ctx.r13));
    asm("mov %0, %%r14\n" :: "m"(next->ctx.r14));
    asm("mov %0, %%r15\n" :: "m"(next->ctx.r15));

    printf("Switched from coroutine %d to %d\n", prev->cid, next->cid);

}

// 协程1函数
void coroutine_func_1(void)
{
 {
        printf("Coroutine 1 is running!\n");
//        sleep(1);
        switch_context(&coroutines[0], &coroutines[1]);
    }
}

// 协程2函数
void coroutine_func_2(void)
{
    {
        printf("Coroutine 2 is running!\n");
//        sleep(1);
        switch_context(&coroutines[1], &coroutines[0]);
    }
}

int main()
{
    // 分配协程栈，并设置初始堆栈指针
    coroutines[0].stack = (char *)malloc(STACK_SIZE);
    coroutines[0].ctx.rsp = (unsigned long)(coroutines[0].stack + STACK_SIZE);
    coroutines[1].stack = (char *)malloc(STACK_SIZE);
    coroutines[1].ctx.rsp = (unsigned long)(coroutines[1].stack + STACK_SIZE);

    // 初始化协程结构体信息
    coroutines[0].cid = 0;
    coroutines[0].func = coroutine_func_1;
    coroutines[1].cid = 1;
    coroutines[1].func = coroutine_func_2;

    // 设置主协程，当前协程为协程0
    current = &coroutines[0];

    // 切换到第一个协程函数
    current->func();

    // 释放协程栈内存
    free(coroutines[0].stack);
    free(coroutines[1].stack);

    return 0;
}