//
// Created by 抑~风 on 2023/1/26.
//

#include "yaml-cpp/yaml.h"
#include <iostream>
#include <fstream>
#include "config.h"
#include "singleton.h"
using namespace std;
using namespace CWJ_CO_NET;


static auto val = GET_CONFIG_MGR()->lookup<int>("key",123,"this is a int");

void test_yaml();

int main() {
    test_yaml();

    cout<<val->getMVal()<<endl;
}

void test_yaml() {
    YAML::Node config = YAML::LoadFile("test.yaml");

    const string username = config["value1"].as<string>();
    const string password = config["value2"].as<string>();

    YAML::Node node = config["list"];

    for(auto i=0;i<node.size();i++){
        cout<<node[i]<<endl;
    }
}
