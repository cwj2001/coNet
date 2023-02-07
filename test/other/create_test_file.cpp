//
// Created by 抑~风 on 2023/2/6.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "cassert"
using namespace std;

const string filename = "test_data.txt";

int main(){

    ofstream out(filename);
    assert(out);
    string str;
    for(int i=0;i<100000;i++){
        str.clear();
        for(int j=0;j<100+(rand()%900);j++){
            str.push_back(('0'+(rand()%10)));
        }
        cout<<str<<endl;
        out<<str<<endl;
    }
}