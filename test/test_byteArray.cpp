//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "byteArray.h"
#include "util.h"
#include "macro.h"

using namespace std;
using namespace CWJ_CO_NET;

int main(){

    ByteArray buf(10);

    string str;

    for(auto i=0;i<100;i++) {
        str.clear();
        for (int i = 0; i < 1000; i++) {
            str.push_back(rand() % 10 + '0');
        }

        cout << str << endl;
        buf.write(str);
        auto rt = buf.read(str.size());
        if( rt != str){
            INFO_LOG(GET_ROOT_LOGGER()) << "2.buf.data_len="<<buf.getMDataSize();
            ERROR_LOG(GET_ROOT_LOGGER()) << endl<< rt << endl << str;
            CWJ_ASSERT(false);
        }else{
            INFO_LOG(GET_ROOT_LOGGER()) <<i<<".read ,write success"<< endl<< rt << endl << str;
        }
    }

    for(auto i=0;i<100;i++) {
        string ttt = "server-";
        str.clear();
        for (int j = 0; j < 1000; j++) {
            str.push_back(rand() % 10 + '0');
        }
        buf.write(ttt);
        vector<iovec>write_ios;
        buf.getWriteBuffers(write_ios,1000);
        int ind = 0;
        for(auto a : write_ios){
            memcpy(a.iov_base,str.c_str()+ind,a.iov_len);
            ind += a.iov_len;
        }
        INFO_LOG(GET_ROOT_LOGGER()) << "1.buf len="<<buf.getMDataSize();
        vector<iovec>read_ios;
        string tmp;
        buf.getReadBuffers(read_ios,str.size()+ttt.size());
        INFO_LOG(GET_ROOT_LOGGER()) << "2.buf len="<<buf.getMDataSize();
        for(int k=0;k<read_ios.size();k++){
            auto& a = read_ios[k];
            tmp += string((char*)a.iov_base,a.iov_len);
//            CWJ_ASSERT(a.iov_base == write_ios[k].iov_base);
        }
        INFO_LOG(GET_ROOT_LOGGER()) << "tmp.len="<<tmp.size();
        if( tmp != (ttt+str)){
            INFO_LOG(GET_ROOT_LOGGER()) << "buf.data_len="<<buf.getMDataSize();
            ERROR_LOG(GET_ROOT_LOGGER()) << endl<< tmp << endl << (ttt+str);
            CWJ_ASSERT(false);
        }else{
            INFO_LOG(GET_ROOT_LOGGER()) <<i<<".get read write success"<< endl<< tmp << endl << (ttt+str);
        }
    }

    cout<<"Hello"<<endl;
    return 0;
}

