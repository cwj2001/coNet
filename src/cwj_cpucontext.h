//
// Created by 抑~风 on 2023-04-17.
//

#ifndef CWJ_CO_NET_CWJ_CPUCONTEXT_H
#define CWJ_CO_NET_CWJ_CPUCONTEXT_H

// 该模块当前不可用

// 封装cpu上下文的模块

namespace CWJ_CO_NET {
    struct CpuCtx_t {
        void *esp; //
        void *ebp;
        void *eip;
        void *edi;
        void *esi;
        void *ebx;
        void *r1;
        void *r2;
        void *r3;
        void *r4;
        void *r5;
    };

    void swapCpuCtx(CpuCtx_t *new_ctx, CpuCtx_t *cur_ctx);

}

#endif //CWJ_CO_NET_CWJ_CPUCONTEXT_H
