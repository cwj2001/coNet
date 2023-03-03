//
// Created by 抑~风 on 2023-03-02.
//

#ifndef CWJ_CO_NET_HTTP_SERVLET_H
#define CWJ_CO_NET_HTTP_SERVLET_H

#include <functional>
#include <unordered_map>
#include <memory>
#include <string>
#include "http.h"
#include "mutex.h"
namespace CWJ_CO_NET{
    namespace http{
        class Servlet{
        public:
            using ptr = std::shared_ptr<Servlet>;
            Servlet(const std::string &mName);

            virtual void handle(HttpRequest::ptr req,HttpResponse::ptr resp) = 0;
            virtual ~Servlet() = default;

            const std::string &getMName() const;

            void setMName(const std::string &mName);

        private:
            std::string m_name;
        };

        class FuncServlet: public Servlet{
        public:
            using CallBack = std::function<void(HttpRequest::ptr req,HttpResponse::ptr resp)>;

            FuncServlet(const std::string &mName, const CallBack &cb);

            void handle(HttpRequest::ptr req, HttpResponse::ptr resp) override;

        private:
            CallBack  m_cb;
        };

        class ServletDispatch{
        public:
            using Mutex = RWMutex;
            using CallBack = std::function<void(HttpRequest::ptr req,HttpResponse::ptr resp)>;

            ServletDispatch();

            void handle(HttpRequest::ptr req, HttpResponse::ptr resp);
            bool addServlet(const std::string &path,const CallBack &cb);
            bool delServlet(const std::string &path);
            bool addServlet(Servlet::ptr s);
            bool delServlet(Servlet::ptr s);

            bool addGlobServlet(const std::string &path,const CallBack &cb);
            bool delGlobServlet(const std::string &path);
            bool addGlobServlet(Servlet::ptr s);
            bool delGlobServlet(Servlet::ptr s);

            void updateDefaultServlet(const CallBack &cb);


        private:

            std::unordered_map<std::string,Servlet::ptr> m_datas;
            std::unordered_map<std::string,Servlet::ptr> m_globs;
            Servlet::ptr m_default;

        };

    }
}

#endif //CWJ_CO_NET_HTTP_SERVLET_H
