#pragma once
#include <stdint.h>
#include <stddef.h>

// we use explicit functions to make it easy to change to a different hashing
// function, since FNV-1a might not be suitable for reasons i'm not aware of

typedef uint32_t Hash;

void hash_init(Hash* h);
void hash_feed(Hash* h, size_t len, void* buffer);
Hash hash_result(Hash* h);
