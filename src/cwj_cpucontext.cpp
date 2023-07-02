//
// Created by 抑~风 on 2023-04-17.
//
#include "cwj_cpucontext.h"


namespace CWJ_CO_NET {

    void swapCpuCtx(CpuCtx_t *new_ctx, CpuCtx_t *cur_ctx) {

#ifdef __i386__
    "movl 8(%esp), %edx      # fs->%edx         \n"
    "movl %esp, 0(%edx)      # save esp         \n"
    "movl %ebp, 4(%edx)      # save ebp         \n"
    "movl (%esp), %eax       # save eip         \n"
    "movl %eax, 8(%edx)                         \n"
    "movl %ebx, 12(%edx)     # save ebx,esi,edi \n"
    "movl %esi, 16(%edx)                        \n"
    "movl %edi, 20(%edx)                        \n"
    "movl 4(%esp), %edx      # ts->%edx         \n"
    "movl 20(%edx), %edi     # restore ebx,esi,edi      \n"
    "movl 16(%edx), %esi                                \n"
    "movl 12(%edx), %ebx                                \n"
    "movl 0(%edx), %esp      # restore esp              \n"
    "movl 4(%edx), %ebp      # restore ebp              \n"
    "movl 8(%edx), %eax      # restore eip              \n"
    "movl %eax, (%esp)                                  \n"
    "ret                                                \n"
    );
#elif defined(__x86_64__)

        __asm__ (
        "       movq %rsp, 0(%rsi)      # save stack_pointer     \n"
        "       movq %rbp, 8(%rsi)      # save frame_pointer     \n"
        "       movq (%rsp), %rax       # save insn_pointer      \n"
        "       movq %rax, 16(%rsi)                              \n"
        "       movq %rbx, 24(%rsi)     # save rbx,r12-r15       \n"
        "       movq %r12, 32(%rsi)                              \n"
        "       movq %r13, 40(%rsi)                              \n"
        "       movq %r14, 48(%rsi)                              \n"
        "       movq %r15, 56(%rsi)                              \n"
        "       movq 56(%rdi), %r15                              \n"
        "       movq 48(%rdi), %r14                              \n"
        "       movq 40(%rdi), %r13     # restore rbx,r12-r15    \n"
        "       movq 32(%rdi), %r12                              \n"
        "       movq 24(%rdi), %rbx                              \n"
        "       movq 8(%rdi), %rbp      # restore frame_pointer  \n"
        "       movq 0(%rdi), %rsp      # restore stack_pointer  \n"
        "       movq 16(%rdi), %rax     # restore insn_pointer   \n"
        "       movq %rax, (%rsp)                                \n"
        "       ret                                              \n"
        );
#endif

    }

}
