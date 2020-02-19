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
  // 5.2.25 NVDIMM Firmware Interface Table (NFIT)
  // Table 5-131 NFIT Structure Types
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
    uint16_t type;
    uint16_t length;
    uint16_t spa_range_structure_index;
    uint16_t flags;
    uint32_t reserved;
    uint32_t proximity_domain;
    uint64_t address_range_type_guid[2];
    uint64_t spa_base;
    uint64_t spa_length;
    uint64_t address_range_memory_mapping_attribute;

    static constexpr GUID kByteAddressablePersistentMemory = {
        0x66F0D379,
        0xB4F3,
        0x4074,
        {0xAC, 0x43, 0x0D, 0x33, 0x18, 0xB7, 0x8C, 0xDB}};

    static constexpr GUID kNVDIMMControlRegion = {
        0x92F701F6,
        0x13B4,
        0x405D,
        {0x91, 0x0B, 0x29, 0x93, 0x67, 0xE8, 0x23, 0x4C}};
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

void PrintStringAndHex64(const char *s, const volatile void *data) {
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
  while (n) {
    if (*s1 != *s2) return *s1 - *s2;
    n--;
    s1++;
    s2++;
  }
  return 0;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 || *s2) {
    if (*s1 != *s2) return *s1 - *s2;
    s1++;
    s2++;
  }
  return 0;
}

NFIT *LookupNFITFromXSDT(XSDT *xsdt) {
  const int kNumOfXSDTEntries = (xsdt->length - offsetof(XSDT, entry)) >> 3;
  PrintStringAndHex64("# of XSDT entries", kNumOfXSDTEntries);
  for (int i = 0; i < kNumOfXSDTEntries; i++) {
    const char *signature = static_cast<const char *>(xsdt->entry[i]);
    if (strncmp(signature, "NFIT", 4) == 0) {
      return static_cast<NFIT *>(xsdt->entry[i]);
    }
  }
  return nullptr;
}

static void ShowNFIT_PrintMemoryMappingAttr(uint64_t attr) {
  PrintString("  attr: ");
  PrintU64AsHex(attr);
  PrintString(" =");
  if (attr & 0x10000) PrintString(" MORE_RELIABLE");
  if (attr & 0x08000) PrintString(" NV");
  if (attr & 0x04000) PrintString(" XP");
  if (attr & 0x02000) PrintString(" RP");
  if (attr & 0x01000) PrintString(" WP");
  if (attr & 0x00010) PrintString(" UCE");
  if (attr & 0x00008) PrintString(" WB");
  if (attr & 0x00004) PrintString(" WT");
  if (attr & 0x00002) PrintString(" WC");
  if (attr & 0x00001) PrintString(" UC");
  PrintChar('\n');
}

void PutGUID(const GUID *guid) {
  const uint64_t *g = reinterpret_cast<const uint64_t *>(guid);
  PrintU64AsHex(g[0]);
  PrintChar('_');
  PrintU64AsHex(g[1]);
}

static void ShowNFIT_PrintMemoryTypeGUID(NFIT::SPARange *spa) {
  GUID *type_guid = reinterpret_cast<GUID *>(&spa->address_range_type_guid);
  PrintString("  type:");
  if (IsEqualGUID(type_guid, &NFIT::SPARange::kByteAddressablePersistentMemory))
    PrintString(" ByteAddressablePersistentMemory");
  else if (IsEqualGUID(type_guid, &NFIT::SPARange::kNVDIMMControlRegion))
    PrintString(" NVDIMMControlRegion");
  else
    PutGUID(type_guid);
  PrintChar('\n');
}

void PrintNFIT(NFIT &nfit) {
  PrintString("NFIT found\n");
  PrintStringAndHex64("NFIT Size in bytes", nfit.length);
  for (auto &it : nfit) {
    switch (it.type) {
      case NFIT::Entry::kTypeSPARangeStructure: {
        NFIT::SPARange *spa_range = reinterpret_cast<NFIT::SPARange *>(&it);
        PrintStringAndHex64("SPARange #", spa_range->spa_range_structure_index);
        PrintStringAndHex64("  Base", spa_range->spa_base);
        PrintStringAndHex64("  Length", spa_range->spa_length);
        ShowNFIT_PrintMemoryMappingAttr(
            spa_range->address_range_memory_mapping_attribute);
        ShowNFIT_PrintMemoryTypeGUID(spa_range);
      } break;
      case NFIT::Entry::kTypeNVDIMMRegionMappingStructure: {
        NFIT::RegionMapping *rmap =
            reinterpret_cast<NFIT::RegionMapping *>(&it);
        PrintString("Region Mapping\n");
        PrintStringAndHex64("  NFIT Device Handle #", rmap->nfit_device_handle);
        PrintStringAndHex64("  NVDIMM phys ID", rmap->nvdimm_physical_id);
        PrintStringAndHex64("  NVDIMM region ID", rmap->nvdimm_region_id);
        PrintStringAndHex64("  SPARange struct index",
                            rmap->spa_range_structure_index);
        PrintStringAndHex64("  ControlRegion struct index",
                            rmap->nvdimm_control_region_struct_index);
        PrintStringAndHex64("  region size", rmap->nvdimm_region_size);
        PrintStringAndHex64("  region offset", rmap->region_offset);
        PrintStringAndHex64("  NVDIMM phys addr region base",
                            rmap->nvdimm_physical_address_region_base);
        PrintStringAndHex64("  NVDIMM interleave_structure_index",
                            rmap->interleave_structure_index);
        PrintStringAndHex64("  NVDIMM interleave ways", rmap->interleave_ways);
        PrintStringAndHex64("  NVDIMM state flags", rmap->nvdimm_state_flags);
      } break;
      case NFIT::Entry::kTypeInterleaveStructure: {
        NFIT::InterleaveStructure *interleave =
            reinterpret_cast<NFIT::InterleaveStructure *>(&it);
        PrintStringAndHex64("Interleave Struct #",
                            interleave->interleave_struct_index);
        PrintStringAndHex64("  Line size (in bytes)", interleave->line_size);
        PrintString("  Lines = ");
        PrintU64AsHex(interleave->num_of_lines_described);
        PrintString(" :");
        for (uint32_t line_index = 0;
             line_index < interleave->num_of_lines_described; line_index++) {
          PrintString(" +");
          PrintU64AsHex(interleave->line_offsets[line_index]);
        }
        PrintChar('\n');
      } break;
      case NFIT::Entry::kTypeNVDIMMControlRegionStructure: {
        NFIT::ControlRegionStruct *ctrl_region =
            reinterpret_cast<NFIT::ControlRegionStruct *>(&it);
        PrintStringAndHex64("Control Region Struct #",
                            ctrl_region->nvdimm_control_region_struct_index);
      } break;
      case NFIT::Entry::kTypeFlushHintAddressStructure: {
        NFIT::FlushHintAddressStructure *flush_hint =
            reinterpret_cast<NFIT::FlushHintAddressStructure *>(&it);
        PrintStringAndHex64("Flush Hint Addresses for NFIT Device handle",
                            flush_hint->nfit_device_handle);
        PrintString("  Addrs = ");
        PrintU64AsHex(flush_hint->num_of_flush_hint_addresses);
        PrintString(" :");
        for (uint32_t index = 0;
             index < flush_hint->num_of_flush_hint_addresses; index++) {
          PrintString(" @");
          PrintU64AsHex(flush_hint->flush_hint_addresses[index]);
        }
        PrintChar('\n');
      } break;
      case NFIT::Entry::kTypePlatformCapabilitiesStructure: {
        NFIT::PlatformCapabilities *caps =
            reinterpret_cast<NFIT::PlatformCapabilities *>(&it);
        const int cap_shift = 31 - caps->highest_valid_cap_bit;
        const uint32_t cap_bits =
            ((caps->capabilities << cap_shift) & 0xFFFF'FFFFUL) >> cap_shift;
        PrintString("Platform Capabilities\n");
        PrintStringAndHex64("  Flush CPU Cache when power loss",
                            cap_bits & 0b001);
        PrintStringAndHex64("  Flush Memory Controller when power loss",
                            cap_bits & 0b010);
        PrintStringAndHex64("  Hardware Mirroring Support", cap_bits & 0b100);
      } break;
    }
  }
  PrintString("NFIT end\n");
}

NFIT &LookupNFIT(SystemTable *system_table) {
  RSDPStructure *rsdp = nullptr;

  PrintStringAndHex64("# of table entries",
                      system_table->number_of_table_entries);
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

  NFIT *nfit = LookupNFITFromXSDT(xsdt);
  if (!nfit) {
    PrintString("NFIT not found!\n");
    Die();
  }
  PrintStringAndHex64("NFIT is at", nfit);
  PrintString("  NFIT Signature: ");
  PrintStringWithSize(nfit->signature, 4);
  PrintChar('\n');

  return *nfit;
}

void PrintSPARange(NFIT::SPARange &spa_range) {
  PrintStringAndHex64("SPARange #", spa_range.spa_range_structure_index);
  PrintStringAndHex64("  Base", spa_range.spa_base);
  PrintStringAndHex64("  Length", spa_range.spa_length);
}

void PrintNFITEntries(NFIT &nfit) {
  for (auto &it : nfit) {
    if (it.type != NFIT::Entry::kTypeSPARangeStructure) {
      continue;
    }
    NFIT::SPARange &spa_range = *reinterpret_cast<NFIT::SPARange *>(&it);
    if (IsEqualGUID(
            reinterpret_cast<GUID *>(&spa_range.address_range_type_guid),
            &NFIT::SPARange::kByteAddressablePersistentMemory)) {
      PrintSPARange(spa_range);
      continue;
    }
  }
}

NFIT::SPARange *GetFirstSPARangeOfByteAddressablePMEM(NFIT &nfit) {
  for (auto &it : nfit) {
    if (it.type != NFIT::Entry::kTypeSPARangeStructure) {
      continue;
    }
    NFIT::SPARange &spa_range = *reinterpret_cast<NFIT::SPARange *>(&it);
    if (IsEqualGUID(
            reinterpret_cast<GUID *>(&spa_range.address_range_type_guid),
            &NFIT::SPARange::kByteAddressablePersistentMemory)) {
      return &spa_range;
    }
  }
  return nullptr;
}

static inline void DoCLWB(volatile void *__p)
{
    asm volatile("clwb %0" : "+m"(*(volatile char*)__p));
}

void RunCommand(const char *input, NFIT &nfit) {
  if (strcmp(input, "write1") == 0) {
    NFIT::SPARange &spa_range = *GetFirstSPARangeOfByteAddressablePMEM(nfit);
    PrintString("First spa range:\n");
    PrintSPARange(spa_range);
    volatile uint64_t *p =
        reinterpret_cast<volatile uint64_t *>(spa_range.spa_base);
    PrintStringAndHex64("Read value before write", *p);
    PrintStringAndHex64("Write 1 at", p);
    *p = 1;
    PrintStringAndHex64("Read value after write ", *p);
    return;
  }
  if (strcmp(input, "write0") == 0) {
    NFIT::SPARange &spa_range = *GetFirstSPARangeOfByteAddressablePMEM(nfit);
    PrintString("First spa range:\n");
    PrintSPARange(spa_range);
    volatile uint64_t *p =
        reinterpret_cast<volatile uint64_t *>(spa_range.spa_base);
    PrintStringAndHex64("Read value before write", *p);
    PrintStringAndHex64("Write 0 at", p);
    *p = 0;
    PrintStringAndHex64("Read value after write ", *p);
    return;
  }
  if (strcmp(input, "clwb") == 0) {
    NFIT::SPARange &spa_range = *GetFirstSPARangeOfByteAddressablePMEM(nfit);
    PrintString("First spa range:\n");
    PrintSPARange(spa_range);
    volatile uint64_t *p =
        reinterpret_cast<volatile uint64_t *>(spa_range.spa_base);
    PrintStringAndHex64("Read value before write", *p);
    PrintStringAndHex64("CLWB on", p);
    DoCLWB(p);
    PrintStringAndHex64("Read value after write ", *p);
    return;
  }
  if (strcmp(input, "show spa") == 0) {
    PrintNFITEntries(nfit);
    return;
  }
  PrintString("Available commands:\n");
}

void efi_main(Handle image_handle, SystemTable *system_table) {
  efi_system_table = system_table;

  PrintString("Hello NVDIMM Playground with efi!\n");

  NFIT &nfit = LookupNFIT(system_table);
  PrintNFIT(nfit);

  constexpr int kKeyBufLen = 16;
  char cmd[kKeyBufLen];
  int cmd_idx = 0;
  for (char c = '\r';; c = EFIGetChar()) {
    if (c == '\r') {
      cmd[cmd_idx] = 0;
      PrintString("\n");
      if (*cmd) RunCommand(cmd, nfit);
      PrintString("> ");
      cmd_idx = 0;
      continue;
    }
    if (c == '\b') {
      if (cmd_idx == 0) continue;
      EFIPutChar(c);
      cmd_idx--;
      continue;
    }
    if (cmd_idx == kKeyBufLen - 1) continue;
    EFIPutChar(c);
    cmd[cmd_idx] = c;
    cmd_idx++;
  };
};
