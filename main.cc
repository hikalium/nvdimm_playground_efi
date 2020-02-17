#include "efi.h"

void EFIPutChar(wchar_t c) {
  wchar_t buf[2];
  buf[0] = c;
  buf[1] = 0;
  efi_system_table->con_out->output_string(
      efi_system_table->con_out, buf);
}

char NibbleToHexChar(uint8_t nibble) {
  if (nibble < 10)
    return nibble + '0';
  return nibble - 10 + 'A';
}

void PrintByte(uint8_t data) {
  EFIPutChar(NibbleToHexChar(data >> 4));
  EFIPutChar(NibbleToHexChar(data & 0xF));
  EFIPutChar('\r');
  EFIPutChar('\n');
}

void puts(const char *s) {
  while (*s) {
    EFIPutChar(*s);
    s++;
  }
}

extern "C" uint8_t ReadIOPort8(uint16_t);

void efi_main(Handle image_handle, SystemTable *system_table) {
  efi_system_table = system_table;
  puts("Hello hikalium\n");
  for (;;) {}
    ;
};
