#include <stdint.h>
#include <stddef.h>

#include "hash.h"

// this is an implementation of the FNV-1a 32 bit hash
// based off of http://www.isthe.com/chongo/src/fnv/hash_32a.c

#define FNV_PRIME ((Hash) 0x01000193)
#define FNV_START ((Hash) 0x811c9dc5)

void hash_init(Hash* h) {
  *h = FNV_START;
}

void hash_feed(Hash* h, size_t len, void* buffer) {
  uint8_t *cursor = buffer;
  uint8_t *end = cursor + len;

  while (cursor < end) {
    *h ^= (Hash) *cursor++;
    *h *= FNV_PRIME;
  }
}


Hash hash_result(Hash* h) {
  return *h;
}
