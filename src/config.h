//
// Created by 抑~风 on 2023/1/26.
//

#ifndef CWJ_CO_NET_CONFIG_H
#define CWJ_CO_NET_CONFIG_H

#include <memory>
#include <string>
#include <unordered_map>
#include "log.h"
#include "singleton.h"
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

        void setMKey(const std::string &mKey) { m_key = mKey; }

        const std::string &getMDescription() const { return m_description; }

        void setMDescription(const std::string &mDescription) { m_description = mDescription; }


    private:
        std::string m_key;
        std::string m_description;
    };

    template<typename T>
    class ConfigVar: public ConfigVarBase{
    public:
        using ptr = std::shared_ptr<ConfigVar<T>>;

        ~ConfigVar(){};

        ConfigVar(const std::string &mKey, const std::string &mDescription, T mVal) : ConfigVarBase(mKey, mDescription),
                                                                                      m_val(mVal) {}

        T getMVal() const {
            return m_val;
        }

        void setMVal(T mVal) {
            m_val = mVal;
        }

    private:
        T m_val;
    };


    class ConfigManager{
    public:
        using ptr = std::shared_ptr<ConfigManager>;
        friend class Singleton<ConfigManager>;



        template<typename T>
        typename ConfigVar<T>::ptr lookup(const std::string& key);

        template<typename T>
        typename ConfigVar<T>::ptr lookup(const std::string& key,const T& val,const std::string& desc);
    private:
        ConfigManager() {};
        std::unordered_map<std::string,ConfigVarBase::ptr> m_cfm;
    };

    using SingleConfigMgr = Singleton<ConfigManager>;

    template<typename T>
    typename ConfigVar<T>::ptr ConfigManager::lookup(const std::string &key, const T &val, const std::string &desc) {

        if(!m_cfm.count(key)){
            m_cfm.emplace(key,ConfigVarBase::ptr(new ConfigVar<T>(key,desc,val)));
            WARN_LOG(GET_ROOT_LOGGER())<<" ConfigVar key : "<<key<<" not exists "<<std::endl;
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
