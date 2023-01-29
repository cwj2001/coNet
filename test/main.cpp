#include <iostream>
#include "util.h"
#include "config.h"
#include "fstream"
#include <vector>
using namespace std;
using namespace CWJ_CO_NET;



int main() {
//    std::cout << "Hello, World!" << std::endl;
//    cout<<TypeToName<ConfigVarBase>()<<endl;
//    cout<<GetAbsolutePath("name")<<endl;
//    vector<string>files;
////    GetAllSameSuffixFile(files,"/tmp/tmp.K52NWsnQH1/cmake-build-debug-",".yaml");
//    cout<<endl;
//    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");

    cout<<YAML::LoadFile("/home/cwj2001/cwj/myCppProject/config/test_array.yaml")<<endl;
    cout<<YAML::LoadFile("/home/cwj2001/cwj/myCppProject/config/logger_config.yaml")<<endl;


//    ifstream in (/home/cwj2001/cwj/myCppProject/config/test.yaml);


//    for(auto &a : files)    cout<<a<<endl;

    return 0;
}
