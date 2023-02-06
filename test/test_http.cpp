//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "util.h"
#include "http/http.h"
using namespace std;
using namespace CWJ_CO_NET;

int main(){

    HttpRequest req;
    req.setMUri("/name/uri?name=12&val=cwj#sdvr");
    cout<<req.getParam("name","20")<<endl;
    cout<<req.getParam("val","www")<<endl;
    cout<<"Hello"<<endl;
    return 0;
}

