#ifndef __SIMPLECGRA_BENCHMARK__

#define UNROLL_THIS_LOOP _Pragma("clang loop unroll(enable)")
#define KEEP_THIS_LOOP   _Pragma("clang loop unroll(disable)")

__attribute__((noinline)) void __loop__(); 

#endif
