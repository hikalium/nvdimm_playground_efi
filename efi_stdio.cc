#include "efi.h"

void EFIPutChar(wchar_t c) {
  wchar_t buf[2];
  buf[0] = c;
  buf[1] = 0;
  efi_system_table->con_out->output_string(efi_system_table->con_out, buf);
}

char EFIGetChar() {
  InputKey key;
  while (true) {
    uint64_t status = efi_system_table->con_in->read_key_stroke(
        efi_system_table->con_in, &key);
    if (status == 0) break;
    asm volatile("pause");
  }
  return key.unicode_char;
}

void PrintChar(char c) {
  if (c == '\n') {
    EFIPutChar('\r');
    EFIPutChar('\n');
    return;
  }
  EFIPutChar(c);
}

void PrintString(const char *s) {
  while (*s) {
    PrintChar(*s);
    s++;
  }
}

void PrintStringWithSize(const char *s, int n) {
  for (int i = 0; i < n; i++) {
    PrintChar(s[i]);
  }
}

static char NibbleToHexChar(uint8_t nibble) {
  if (nibble < 10) return nibble + '0';
  return nibble - 10 + 'A';
}

void PrintU8AsHex(uint8_t data) {
  EFIPutChar(NibbleToHexChar(data >> 4));
  EFIPutChar(NibbleToHexChar(data & 0xF));
}

void PrintU64AsHex(uint64_t data) {
  for (int i = 7; i >= 0; i--) {
    PrintU8AsHex(data >> (i * 8));
  }
}

void PrintStringAndHex64(const char *s, uint64_t data) {
  PrintString(s);
  PrintString(": 0x");
  PrintU64AsHex(data);
  PrintString("\n");
}

void PrintStringAndHex64(const char *s, const void *data) {
  PrintStringAndHex64(s, reinterpret_cast<uint64_t>(data));
}

void PrintStringAndHex64(const char *s, const volatile void *data) {
  PrintStringAndHex64(s, reinterpret_cast<uint64_t>(data));
}

