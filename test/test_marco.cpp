#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <execinfo.h>
#include <assert.h>
#include <cxxabi.h>
#include <unistd.h>
#include "macro.h"
#include "util.h"
#include "macro.h"
using namespace std;
using namespace CWJ_CO_NET;

void test(int n = 10){
    if(n)   test(n-1);
    else CWJ_ASSERT_MGS(false,"test assert");
}

int main(int argc, char* argv[])
{
    test();
    return 0;
}
