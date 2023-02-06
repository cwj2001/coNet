//
// Created by 抑~风 on 2023/2/6.
//
#include "http.h"
#include "macro.h"
#include "log.h"
namespace CWJ_CO_NET{
static auto g_logger = GET_LOGGER("httpserver");

    HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, string) \
    if(strcmp(#string, m.c_str()) == 0) { \
        return HttpMethod::name; \
    }
        HTTP_METHOD_MAP(XX);
#undef XX
        return HttpMethod::INVALID_METHOD;
    }

    HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string) \
    if(strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name; \
    }
        HTTP_METHOD_MAP(XX);
#undef XX
        return HttpMethod::INVALID_METHOD;
    }

    static const char* s_method_string[] = {
#define XX(num, name, string) #string,
            HTTP_METHOD_MAP(XX)
#undef XX
    };

    const char* HttpMethodToString(const HttpMethod& m) {
        uint32_t idx = (uint32_t)m;
        if(idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
            return "<unknown>";
        }
        return s_method_string[idx];
    }

    const char* HttpStatusToString(const HttpStatus& s) {
        switch(s) {
#define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;
            HTTP_STATUS_MAP(XX);
#undef XX
            default:
                return "<unknown>";
        }
    }


    std::string HttpRequest::getParam(const std::string &key, const std::string &def) {
        if(!hasParam(key)) {
            m_params[key] = def;
            INFO_LOG(g_logger)<<def<<" == " <<key<<" | "<<m_params[key] << " == ";
        }
        return m_params[key];
    }

    std::string HttpRequest::getHeader(const std::string &key, const std::string &def) {
        if(!hasHeader(key)) m_headers[key] = def;
        return m_headers[key];
    }

    std::string HttpRequest::getCookie(const std::string &key, const std::string &def) {
        if(!hasCookie(key)) m_cookies[key] = def;
        return m_cookies[key];
    }

    void HttpRequest::handleUri(const std::string &uri) {
        auto start = uri.find('?');
        auto ed= uri.find('#');
        ed = (ed == std::string::npos ? uri.size():ed);
        std::string key,tmp;
        INFO_LOG(g_logger) << start << " "<<ed;
        for(auto i=start+1;i<ed;i++){
            if(uri[i] == '='){
                key = std::move(tmp);
                tmp.clear();
            }else if(uri[i] == '&'){
                m_params[key] = std::move(tmp);
                tmp.clear();
            }else{
                tmp.push_back(uri[i]);
                INFO_LOG(g_logger)<<tmp;
            }
        }
        if(tmp.size() && key.size()){
            m_params[key] = tmp;
        }
    }

    std::ostream& HttpRequest::dump(std::ostream &os) {
        return os;
    }
}
