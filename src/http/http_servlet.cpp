//
// Created by 抑~风 on 2023-03-02.
//

#include <fnmatch.h>

#include "http_servlet.h"
#include "macro.h"

namespace CWJ_CO_NET{
    namespace http{

        static auto g_logger = GET_LOGGER("http_server");

        Servlet::Servlet(const std::string &mName) : m_name(mName) {

        }

        const std::string &Servlet::getMName() const {
            return m_name;
        }

        void Servlet::setMName(const std::string &mName) {
            m_name = mName;
        }

        void FuncServlet::handle(HttpRequest::ptr req, HttpResponse::ptr resp) {
            m_cb(req,resp);
        }

        FuncServlet::FuncServlet(const std::string &mName, const FuncServlet::CallBack &cb) : Servlet(mName), m_cb(cb) {}

        void ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr resp) {
            auto url = req->getPath();
            Servlet::ptr p = nullptr;
            if(m_datas.count(url)>0){
                p = m_datas[url];
            }else{
                for(auto a : m_globs){
                    if(!fnmatch(a.first.c_str(),url.c_str(),0)){
                        p = a.second;
                    }
                }
            }
            if(!p){
                p = m_default;
            }

            p->handle(req,resp);

        }

        bool ServletDispatch::addServlet(const std::string &path,const  ServletDispatch::CallBack& cb) {
            bool ret = false;
            if(m_datas.count(path) > 0) ret = true;
            m_datas[path] = Servlet::ptr(new FuncServlet(path,cb));
            return ret;
        }

        bool ServletDispatch::delServlet(const std::string &path) {
            bool ret = false;
            if(m_datas.count(path) > 0) ret = true;
            m_datas.erase(path);
            return ret;
        }

        bool ServletDispatch::addServlet(Servlet::ptr s) {
            bool ret = false;
            if(m_datas.count(s->getMName()) > 0) ret = true;
            m_datas[s->getMName()] = s;
            return ret;
        }

        bool ServletDispatch::delServlet(Servlet::ptr s) {
            bool ret = false;
            if(m_datas.count(s->getMName()) > 0) ret = true;
            m_datas.erase(s->getMName());
            return ret;
        }

        ServletDispatch::ServletDispatch() {
            m_default.reset(new FuncServlet("Not found"
                                            ,[](auto req,HttpResponse::ptr resp){
                WARN_LOG(g_logger) << "path Not found";
                resp->setBody("Not found path");
            }));
        }

        bool ServletDispatch::addGlobServlet(const std::string &path, const ServletDispatch::CallBack &cb) {
            bool ret = 0;
            if(m_globs.count(path)) ret = 1;
            m_globs[path].reset(new FuncServlet(path,cb));
            return ret;
        }

        bool ServletDispatch::delGlobServlet(const std::string &path) {
            bool ret = 0;
            if(m_globs.count(path)) ret = 1,m_globs.erase(path);
            return ret;
        }

        bool ServletDispatch::addGlobServlet(Servlet::ptr s) {
            bool ret = 0;
            auto path = s->getMName();
            if(m_globs.count(path)) ret = 1;
            m_globs[path] = s;
            return ret;
        }

        bool ServletDispatch::delGlobServlet(Servlet::ptr s) {
            bool ret = 0;
            if(m_globs.count(s->getMName())) ret = 1,m_globs.erase(s->getMName());
            return ret;
        }

        void ServletDispatch::updateDefaultServlet(const ServletDispatch::CallBack &cb) {
            m_default.reset(new FuncServlet("Not found",cb));
        }


    }
}