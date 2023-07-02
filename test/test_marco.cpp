#include <assert.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "macro.h"
#include "util.h"
using namespace std;
using namespace CWJ_CO_NET;

void test(int n = 10) {
  if (n)
    test(n - 1);
  else
    CWJ_ASSERT_MGS(false, "test assert");
}

int main(int argc, char* argv[]) {
  // ucontext_t d;
  test();
  return 0;
}
