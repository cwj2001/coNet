//
// Created by 抑~风 on 2023-04-15.
//

#ifndef CWJ_CO_NET_CWJ_SIGNAL_H
#define CWJ_CO_NET_CWJ_SIGNAL_H

#include <memory>
#include <string>
namespace CWJ_CO_NET{
    struct SignalCtx{
        using ptr = std::shared_ptr<SignalCtx>;
        using Handler =  void  (*)(int signo);
        int m_signo;
        std::string m_sig_name;
        std::string m_name;
        Handler m_handler;
    };
}

#endif //CWJ_CO_NET_CWJ_SIGNAL_H
