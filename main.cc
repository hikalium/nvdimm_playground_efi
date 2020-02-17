#define packed_struct struct __attribute__((__packed__))

#include <stddef.h>

#include "efi.h"
#include "guid.h"

struct XSDT;

struct __attribute((__packed__)) RSDPStructure {
  char signature[8];
  uint8_t checksum;
  uint8_t oem_id[6];
  uint8_t revision;
  uint32_t rsdt_address;
  uint32_t length;
  XSDT *xsdt_address;
  uint8_t extended_checksum;
  uint8_t reserved;
};

struct __attribute__((__packed__)) XSDT {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  void *entry[1];
};

packed_struct NFIT {
  packed_struct Entry {
    static const uint16_t kTypeSPARangeStructure = 0x00;
    static const uint16_t kTypeNVDIMMRegionMappingStructure = 0x01;
    static const uint16_t kTypeInterleaveStructure = 0x02;
    static const uint16_t kTypeNVDIMMControlRegionStructure = 0x04;
    static const uint16_t kTypeFlushHintAddressStructure = 0x06;
    static const uint16_t kTypePlatformCapabilitiesStructure = 0x07;

    uint16_t type;
    uint16_t length;
  };

  class Iterator {
   public:
    Iterator(Entry *e) : current_(e){};
    void operator++() {
      current_ = reinterpret_cast<Entry *>(
          reinterpret_cast<uint64_t>(current_) + current_->length);
    }
    Entry &operator*() { return *current_; }
    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
      return lhs.current_ != rhs.current_;
    }

   private:
    Entry *current_;
  };
  Iterator begin() { return Iterator(entry); }
  Iterator end() {
    return Iterator(
        reinterpret_cast<Entry *>(reinterpret_cast<uint64_t>(this) + length));
  }

  packed_struct SPARange {
    // System Physical Address Range
    static const GUID kByteAdressablePersistentMemory;
    static const GUID kNVDIMMControlRegion;

    uint16_t type;
    uint16_t length;
    uint16_t spa_range_structure_index;
    uint16_t flags;
    uint32_t reserved;
    uint32_t proximity_domain;
    uint64_t address_range_type_guid[2];
    uint64_t system_physical_address_range_base;
    uint64_t system_physical_address_range_length;
    uint64_t address_range_memory_mapping_attribute;
  };

  packed_struct RegionMapping {
    uint16_t type;
    uint16_t length;
    uint32_t nfit_device_handle;
    uint16_t nvdimm_physical_id;
    uint16_t nvdimm_region_id;
    uint16_t spa_range_structure_index;
    uint16_t nvdimm_control_region_struct_index;
    uint64_t nvdimm_region_size;
    uint64_t region_offset;
    uint64_t nvdimm_physical_address_region_base;
    uint16_t interleave_structure_index;
    uint16_t interleave_ways;
    uint16_t nvdimm_state_flags;
    uint16_t reserved;
  };
  static_assert(sizeof(RegionMapping) == 48);

  packed_struct InterleaveStructure {
    uint16_t type;
    uint16_t length;
    uint16_t interleave_struct_index;
    uint16_t reserved;
    uint32_t num_of_lines_described;
    uint32_t line_size;
    uint32_t line_offsets[1];
  };
  static_assert(offsetof(InterleaveStructure, line_offsets) == 16);

  packed_struct FlushHintAddressStructure {
    uint16_t type;
    uint16_t length;
    uint32_t nfit_device_handle;
    uint16_t num_of_flush_hint_addresses;
    uint16_t reserved[3];
    uint64_t flush_hint_addresses[1];
  };
  static_assert(offsetof(FlushHintAddressStructure, flush_hint_addresses) ==
                16);

  packed_struct PlatformCapabilities {
    uint16_t type;
    uint16_t length;
    uint8_t highest_valid_cap_bit;
    uint8_t reserved0[3];
    uint32_t capabilities;
    uint32_t reserved1;
  };
  static_assert(sizeof(PlatformCapabilities) == 16);

  packed_struct ControlRegionStruct {
    uint16_t type;
    uint16_t length;
    uint16_t nvdimm_control_region_struct_index;
  };

  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint32_t reserved;
  Entry entry[1];
};
static_assert(offsetof(NFIT, entry) == 40);

void EFIPutChar(wchar_t c) {
  wchar_t buf[2];
  buf[0] = c;
  buf[1] = 0;
  efi_system_table->con_out->output_string(efi_system_table->con_out, buf);
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

char NibbleToHexChar(uint8_t nibble) {
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

static const GUID kACPITableGUID = {
    0x8868e871,
    0xe4f1,
    0x11d3,
    {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

extern "C" uint8_t ReadIOPort8(uint16_t);

[[noreturn]] static void Die(void) {
  PrintString("\nHalted...\n");
  for (;;) {
  }
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while(n) {
    if(*s1 != *s2) return *s1 - *s2;
    n--;
    s1++;
    s2++;
  }
  return 0;
}

NFIT *FindNFIT(XSDT *xsdt) {
  const int kNumOfXSDTEntries = (xsdt->length - offsetof(XSDT, entry)) >> 3;
  PrintStringAndHex64("# of XSDT entries",
                      kNumOfXSDTEntries);
  for (int i = 0; i < kNumOfXSDTEntries; i++) {
    const char *signature = static_cast<const char *>(xsdt->entry[i]);
    if (strncmp(signature, "NFIT", 4) == 0) {
      return static_cast<NFIT *>(xsdt->entry[i]);
    }
  }
  return nullptr;
}

void efi_main(Handle image_handle, SystemTable *system_table) {
  efi_system_table = system_table;
  PrintString("Hello NVDIMM Playground with efi!\n");
  PrintStringAndHex64("# of table entries",
                      system_table->number_of_table_entries);

  RSDPStructure *rsdp = nullptr;

  for (uint64_t i = 0; i < system_table->number_of_table_entries; i++) {
    if (IsEqualGUID(&kACPITableGUID,
                    &system_table->configuration_table[i].vendor_guid)) {
      PrintStringAndHex64("ACPI Table Found at index", i);
      rsdp = reinterpret_cast<RSDPStructure *>(
          system_table->configuration_table[i].vendor_table);
    }
  }
  if (!rsdp) {
    PrintString("RSDP not found!\n");
    Die();
  }
  PrintStringAndHex64("RSDPStructure is at", rsdp);
  PrintString("  RSDPStructure Signature: ");
  PrintStringWithSize(rsdp->signature, 8);
  PrintChar('\n');

  XSDT *xsdt = rsdp->xsdt_address;
  PrintStringAndHex64("XSDT is at", xsdt);
  PrintString("  XSDT Signature: ");
  PrintStringWithSize(xsdt->signature, 4);
  PrintChar('\n');

  NFIT *nfit = FindNFIT(xsdt);
  if(!nfit) {
    PrintString("NFIT not found!\n");
    Die();
  }
  PrintStringAndHex64("NFIT is at", nfit);
  PrintString("  NFIT Signature: ");
  PrintStringWithSize(nfit->signature, 4);
  PrintChar('\n');

  for (;;) {
  };
};
