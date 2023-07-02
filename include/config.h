//
// Created by 抑~风 on 2023/1/26.
//

#ifndef CWJ_CO_NET_CONFIG_H
#define CWJ_CO_NET_CONFIG_H

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "log.h"
#include "singleton.h"
#include "yaml_cast.h"
#include "util.h"

namespace CWJ_CO_NET{


#define GET_CONFIG_MGR() \
    SingleConfigMgr ::GetInstance()


    class ConfigVarBase{
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;

        virtual ~ConfigVarBase(){};

        ConfigVarBase(const std::string &mKey, const std::string &mDescription) : m_key(mKey),
                                                                                  m_description(mDescription) {}

        const std::string &getMKey() const {return m_key;}

        const std::string &getMDescription() const { return m_description; }

        virtual std::string toString() = 0;

        virtual void fromYaml(YAML::Node& n) = 0;

    protected:
        std::string m_key;
        std::string m_description;
    };

    template<typename T
            ,typename toStr = YamlCast<T,std::string>
            ,typename fromYaNo = YamlCast<YAML::Node,T>>
    class ConfigVar: public ConfigVarBase{
    public:
        using ptr = std::shared_ptr<ConfigVar<T>>;
        using CallBack = std::function<void(const T& oldVal,const T& newVal)>;

        ~ConfigVar(){};

        ConfigVar(const std::string &mKey, const std::string &mDescription, T mVal) : ConfigVarBase(mKey, mDescription),
                                                                                      m_val(mVal) {}

        T getMVal() const {
            return m_val;
        }

        void setMVal(T mVal) {
            if(m_val == mVal)   return ;
            for(auto &a: m_cbs){
                a.second(m_val,mVal);
            }
            m_val = std::move(mVal);
        }

        void addCallBack(uint32_t key,CallBack cb);

        bool delCallBack(uint32_t key);

        std::string toString() override;

        void fromYaml(YAML::Node& n) override;

    private:
        T m_val;
        std::unordered_map<uint32_t ,CallBack>m_cbs;
    };

    template<typename T, typename toStr, typename fromYaNo>
    std::string ConfigVar<T, toStr, fromYaNo>::toString() {
        try {
            return toStr()(getMVal());
        }catch (std::exception & e){
            ERROR_LOG(GET_LOGGER("system"))<<"ConfigVar::toString exception :"<<e.what()
            << "convert: " << TypeToName<T>() << " to string  key : "<< m_key ;
        }
        return "";
    }

    template<typename T, typename toStr, typename fromYaNo>
    void ConfigVar<T, toStr, fromYaNo>::fromYaml(YAML::Node &n) {
        try {
            setMVal(fromYaNo()(n));
        }catch (std::exception & e){
            ERROR_LOG(GET_LOGGER("system"))<<"ConfigVar::fromYaml exception :"<<e.what()<<std::endl
                                           << "convert: from YAML to "<<TypeToName<T>()<<"   key : "<< m_key ;
        }
    }

    template<typename T, typename toStr, typename fromYaNo>
    void ConfigVar<T, toStr, fromYaNo>::addCallBack(uint32_t key, ConfigVar::CallBack cb) {
        m_cbs[key] = cb;
    }

    template<typename T, typename toStr, typename fromYaNo>
    bool ConfigVar<T, toStr, fromYaNo>::delCallBack(uint32_t key) {
        return m_cbs.erase(key);
    }


    class ConfigManager{
    public:
        using ptr = std::shared_ptr<ConfigManager>;
        using FileConfigMapType = std::unordered_map<std::string,YAML::Node>;
        using ConfigMapType =  std::unordered_map<std::string,ConfigVarBase::ptr>;

        friend class Singleton<ConfigManager>;

        template<typename T>
        typename ConfigVar<T>::ptr lookup(const std::string& key);

        typename ConfigVarBase::ptr lookupBase(const std::string& key);

        template<typename T>
        typename ConfigVar<T>::ptr lookup(const std::string& key,const T& val,const std::string& desc);


        /**
         * 注意：如果想实现约定大于配置，那么就必须保证该函数程序运行时被调用，而不是在全局变量初始化被调用
         * */
        static void loadYamlFromDir(const std::string & dirName);


    private:
        static void listAllMember(const std::string& prefix
                           ,const YAML::Node& cur
                           ,FileConfigMapType& output);
        static void loadYamlFromFile(const std::string & fileName);

        ConfigManager() { };
    private:
        ConfigMapType m_cfm;
    };

    using SingleConfigMgr = Singleton<ConfigManager>;

    template<typename T>
    typename ConfigVar<T>::ptr ConfigManager::lookup(const std::string &key, const T &val, const std::string &desc) {
//        INFO_LOG(GET_ROOT_LOGGER())<<"lookup";
        if(!m_cfm.count(key)){
                m_cfm.emplace(key,ConfigVarBase::ptr(new ConfigVar<T>(key,desc,val)));
                WARN_LOG(GET_LOGGER("system"))<<" ConfigVar key : "<<key<<" not exists ";
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(m_cfm[key]);
    }

    template<typename T>
    typename ConfigVar<T>::ptr ConfigManager::lookup(const std::string &key) {
        if(m_cfm.count(key))    return std::dynamic_pointer_cast<ConfigVar<T> >(m_cfm[key]);
        else {
            WARN_LOG(GET_ROOT_LOGGER())<<" ConfigVar key : "<<key<<" not exists "<<std::endl;
            return nullptr;
        }
    }

}


#endif //CWJ_CO_NET_CONFIG_H
