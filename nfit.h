
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

