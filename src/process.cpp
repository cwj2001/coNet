//
// Created by 抑~风 on 2023-04-15.
//
#include <signal.h>
#include <cstring>
#include <socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include "util.h"
#include "log.h"
#include "process.h"
#include "macro.h"
#include "cwj_channel.h"


namespace CWJ_CO_NET{

    static pid_t g_pid = GetProcessId();

    int Process::GetProcessId() {
        return g_pid;
    }

    void Process::UpdateProcessId() {
        g_pid = CWJ_CO_NET::GetProcessId();
    }
}

