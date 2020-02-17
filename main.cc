#include "efi.h"
#include "guid.h"

void EFIPutChar(wchar_t c) {
  wchar_t buf[2];
  buf[0] = c;
  buf[1] = 0;
  efi_system_table->con_out->output_string(
      efi_system_table->con_out, buf);
}

void PrintString(const char *s) {
  while (*s) {
    if(*s == '\n') {
      EFIPutChar('\r');
      EFIPutChar('\n');
    } else{
      EFIPutChar(*s);
    }
    s++;
  }
}

char NibbleToHexChar(uint8_t nibble) {
  if (nibble < 10)
    return nibble + '0';
  return nibble - 10 + 'A';
}

void PrintU8AsHex(uint8_t data) {
  EFIPutChar(NibbleToHexChar(data >> 4));
  EFIPutChar(NibbleToHexChar(data & 0xF));
}

void PrintU64AsHex(uint64_t data) {
  for(int i = 7; i >= 0; i--) {
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

static const GUID kACPITableGUID = {
    0x8868e871,
    0xe4f1,
    0x11d3,
    {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

extern "C" uint8_t ReadIOPort8(uint16_t);

[[noreturn]]static void Die(void) {
  PrintString("\nHalted...\n");
  for(;;) {}
}

void efi_main(Handle image_handle, SystemTable *system_table) {
  efi_system_table = system_table;
  PrintString("Hello NVDIMM Playground with efi!\n");
  PrintStringAndHex64("# of table entries", system_table->number_of_table_entries);

  void *rsdp = nullptr;
  
  for(uint64_t i = 0; i < system_table->number_of_table_entries; i++){
    if(IsEqualGUID(&kACPITableGUID, &system_table->configuration_table[i].vendor_guid)) {
    PrintStringAndHex64("ACPI Table Found at index", i);
    rsdp = system_table->configuration_table[i].vendor_table;
    }
  }
  if(!rsdp) {
    Die();
  }
  PrintStringAndHex64("RSDP is at", rsdp);

  for (;;) {}
    ;
};
