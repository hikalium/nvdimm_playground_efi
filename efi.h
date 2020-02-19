#include <stdint.h>

#include "guid.h"

typedef void *Handle;
typedef uint64_t UINTN;

class RuntimeServices;
class BootServices;

struct InputKey {
  uint16_t scan_code;
  wchar_t unicode_char;
};

struct SimpleTextInputProtocol {
  void *reset;
  uint64_t (*read_key_stroke)(SimpleTextInputProtocol *,
                              InputKey *);
  void *wait_for_key;
};

struct TableHeader {
  uint64_t signature;
  uint32_t revision;
  uint32_t header_size;
  uint32_t crc32;
  uint32_t reserved;
};

struct SimpleTextOutputProtocol {
  uint64_t (*reset)(SimpleTextOutputProtocol *,
                            bool);
  uint64_t (*output_string)(SimpleTextOutputProtocol *,
                            const wchar_t *);
  uint64_t (*test_string)(SimpleTextOutputProtocol *,
                          wchar_t *);
  uint64_t (*query_mode)(SimpleTextOutputProtocol *, wchar_t *,
                         uint64_t *columns, uint64_t *rows);
  uint64_t (*set_mode)(SimpleTextOutputProtocol *, uint64_t);
  uint64_t (*set_attribute)(SimpleTextOutputProtocol *,
                            uint64_t Attribute);
  uint64_t (*clear_screen)(SimpleTextOutputProtocol *);
};

struct ConfigurationTable {
  GUID vendor_guid;
  void* vendor_table;
};

struct SystemTable {
  TableHeader header;
  wchar_t *firmware_vendor;
  uint32_t firmware_revision;
  Handle console_in_handle;
  SimpleTextInputProtocol *con_in;
  Handle console_out_handle;
  SimpleTextOutputProtocol *con_out;
  Handle standard_error_handle;
  SimpleTextOutputProtocol *std_err;
  RuntimeServices *runtime_services;
  BootServices *boot_services;
  UINTN number_of_table_entries;
  ConfigurationTable *configuration_table;
};

SystemTable *efi_system_table;

