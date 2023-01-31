//
// Created by 抑~风 on 2023/1/26.
//

#include "yaml-cpp/yaml.h"
#include "config.h"
#include "log.h"
#include "assert.h"
#include "util.h"
#include <vector>


namespace CWJ_CO_NET {

    void CWJ_CO_NET::ConfigManager::listAllMember(const std::string &prefix, const YAML::Node &cur,
                                                  FileConfigMapType &output) {

        if (!prefix.empty()) output[prefix] = cur;
        if (cur.IsMap()) {
            for (auto &p : cur) {
                if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz"
                                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                             "._012345678") != std::string::npos) {
                    WARN_LOG(GET_LOGGER("system")) << "config "
                                                      "file exist invaild prefix :" << prefix;
                    continue;
                }
                listAllMember(prefix.empty() ? p.first.Scalar() : prefix + "." + p.first.Scalar(), p.second, output);
            }
        }


    }

    void CWJ_CO_NET::ConfigManager::loadYamlFromFile(const std::string &fileName) {

        Logger::ptr logger = GET_LOGGER("system");
        YAML::Node root = YAML::LoadFile(fileName);
//        std::cout<<root<<std::endl;
        FileConfigMapType allM;

        listAllMember("", root, allM);

        for(auto& a : allM){

            ConfigVarBase::ptr p = GET_CONFIG_MGR()->lookupBase(a.first);
            if(p){
                p->fromYaml(a.second);
            }
        }
    }


    ConfigVarBase::ptr CWJ_CO_NET::ConfigManager::lookupBase(const std::string &key) {
        if(m_cfm.count(key))    return m_cfm[key];
        else {
//            WARN_LOG(GET_ROOT_LOGGER())<<" ConfigVar key : "<<key<<" not exists "<<std::endl;
            return nullptr;
        }
    }

    void CWJ_CO_NET::ConfigManager::loadYamlFromDir(const std::string &dirName) {

        auto path = GetAbsolutePath(dirName);
        std::vector<std::string>files;
        GetAllSameSuffixFile(files,dirName,".yaml");

        for(const auto& a : files){
            try {
                loadYamlFromFile(a);
                INFO_LOG(GET_LOGGER("system"))<<"ConfigManager::loadYamlFromDir load config file :"
                                              << a << " success ";
            }catch (std::exception& e){
                WARN_LOG(GET_LOGGER("system"))<<"ConfigManager::loadYamlFromDir load config file :"
                << a << " failed " << " as : "<< e.what();
            }
        }
    }
}

