#pragma once

[[noreturn]] void assertion_failed(const char* expr_str, const char* file,
                                   int line);
#undef assert
#define assert(expr) \
  ((void)((expr) || (assertion_failed(#expr, __FILE__, __LINE__), 0)))
