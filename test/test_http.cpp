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
using namespace CWJ_CO_NET::http;

int main(){

    HttpRequest req;
    req.setMethod(HttpMethod::POST);
    req.setPath("/d/myCppWorkPlace/cwj_co_net");
    req.setBody("123456");
    req.setCookie("host","qwerv24");
    req.setHeader("erg5g","wrfgwg4");
    req.setQuery("v=sdfer&veg=efe");
    req.dump(cout);

    cout<<"----------------------"<<endl;

    HttpResponse resp;

    resp.setHeader("dggrt","ergrt");
    resp.setBody("grthtyhny");
    resp.setClose(true);
    resp.setHeader("yukyu","srger");
    resp.dump(cout);

    return 0;
}

