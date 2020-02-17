#pragma once

struct __attribute__((__packed__)) GUID {
  uint32_t id1;
  uint16_t id2;
  uint16_t id3;
  uint8_t id4[8];
};

static inline bool IsEqualGUID(const GUID* guid1, const GUID* guid2) {
  const uint64_t* g1 = reinterpret_cast<const uint64_t*>(guid1);
  const uint64_t* g2 = reinterpret_cast<const uint64_t*>(guid2);
  return g1[0] == g2[0] && g1[1] == g2[1];
}
