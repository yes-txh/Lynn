#ifndef COMMON_CRYPTO_HASH_STRING_HASHES_HPP
#define COMMON_CRYPTO_HASH_STRING_HASHES_HPP

unsigned int ELFhash(const char* url, unsigned int hash_size);
unsigned int HfIp(const char* url,unsigned int hash_size);
unsigned int hf(const char* url, unsigned int hash_size);

#endif // COMMON_CRYPTO_HASH_STRING_HASHES_HPP
