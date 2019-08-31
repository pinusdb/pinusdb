#pragma once

#include "internal.h"

int crash_handler(char const* module, struct _EXCEPTION_POINTERS *ep, DWORD thread_id = 0);

// 相对于 crash handler, 增加了对栈溢出情况的特殊处理，详情请看具体实现代码
int crash_handler_with_stack_overflow(char const* module, struct _EXCEPTION_POINTERS *ep);

// a global exception handler is needed for 
// threads that not use SEH.
LONG WINAPI exception_handler(_EXCEPTION_POINTERS *ExceptionInfo);
//纯虚函数调用异常的处理
void purcall_handler();
// special handler to catch CRT invalid parameter errors, 
// they can not be catched by exception_handler.
void invalid_param_handler(const wchar_t* expression
  , const wchar_t* function
  , const wchar_t* file
  , unsigned int line
  , uintptr_t pReserved);


#define PDB_SEH_BEGIN(main) \
  if (main) { \
    SetUnhandledExceptionFilter(exception_handler); \
    _set_purecall_handler(purcall_handler);  \
    _set_invalid_parameter_handler(invalid_param_handler);  \
  } __try{ do{} while(0)

#define PDB_SEH_END(module, action) \
  }__except(crash_handler_with_stack_overflow(module, GetExceptionInformation())) { \
  if (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) {} action; } \
  do{} while(0)


