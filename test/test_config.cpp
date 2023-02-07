//
// Created by 抑~风 on 2023/1/26.
//


#include <iostream>
#include "config.h"
#include "singleton.h"
#include <string>
#include <vector>
#include "yaml_cast.h"
#include "yaml-cpp/yaml.h"
using namespace std;
using namespace CWJ_CO_NET;
namespace CWJ_CO_NET{


}

auto g_Logger = GET_LOGGER("system");

static auto val = GET_CONFIG_MGR()->lookup<int>("key1 ",123,"this is a int");
static auto g_name_v = GET_CONFIG_MGR()->lookup<std::string>("name",(string)"www","this is name");
static auto g_age_v = GET_CONFIG_MGR()->lookup<int>("age",0,"desc age");
static auto g_course_list = GET_CONFIG_MGR()->lookup<vector<string>>("course",{"1","2","3"},"desc course list");
static auto g_people_a_list = GET_CONFIG_MGR()->lookup<vector<int>>("people.boy.a",{1,2,3},"desc people a list");
static auto g_people_b_map = GET_CONFIG_MGR()->lookup<unordered_map<string,string>>("people.boy.b",{{"c1","1"},{"c2","2"},{"c3","3"}},"desc course list");

void test_yaml();

int main() {

    cout<<"------------------------------------------------"<<endl;
    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");
    INFO_LOG(g_Logger)<<" ============= Scalar ================"<<endl;
    INFO_LOG(g_Logger)<<val->getMVal()<<" "<<g_name_v->getMVal()<<" "<<g_age_v->getMVal()<<endl;
    INFO_LOG(g_Logger)<<" ============= course list ================";
    for(auto &a:g_course_list->getMVal()){
        INFO_LOG(g_Logger)<<a<<" ";
    }
    cout<<endl;

    INFO_LOG(g_Logger)<<" ============= people.boy.a list ================";
    for(auto &a:g_people_a_list->getMVal()){
        ERROR_LOG(g_Logger)<<a<<" ";
    }

    cout<<endl;

    INFO_LOG(g_Logger)<<" ============= map ================";
    for(auto &a:g_people_b_map->getMVal()){
        INFO_LOG(g_Logger)<<a.first<<" "<<a.second<<endl;
    }

    for(int i=0;i<100000;i++){
        INFO_LOG(g_Logger) << "tetet_"+to_string(i);
        ERROR_LOG(g_Logger) << "tetet_"+to_string(i);
    }


}

void test_yaml() {

}
