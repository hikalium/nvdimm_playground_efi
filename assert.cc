#include "efi.h"
#include "efi_stdio.h"

[[noreturn]] static void Die(void) {
  PrintString("\nHalted...\n");
  for (;;) {
    asm volatile("hlt");
  }
}

void assertion_failed(const char* expr_str, const char* file, int line) {
  PrintString("Assertion failed: ");
  PrintString(expr_str);
  PrintString(" at ");
  PrintString(file);
  PrintString(":0x");
  PrintU64AsHex(line);
  PrintString("\n");
  Die();
}
