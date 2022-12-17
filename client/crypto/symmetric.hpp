// Masks the crypto library to provide an high-level abstraction for
// anything related to symmetric encryption, and required by the client.

#ifndef CLIENT_CRYPTO_SYMMETRIC_H
#define CLIENT_CRYPTO_SYMMETRIC_H

#ifdef WIN32
#include <aes.h>
#include <filters.h>
#include <modes.h>
#elif __linux__
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#endif

#include "../tempfile.hpp"

namespace messageu {
namespace crypto {
namespace symmetric {

using CryptoPP::byte;

constexpr auto kKeySize = CryptoPP::AES::DEFAULT_KEYLENGTH;

class Key {
 public:
  // Generates a new key
  Key();

  // Loads a key
  Key(const char key[kKeySize]);

  Key(const Key &other);

  // Exports the key to a tempfile
  void Export(const tempfile::TempFile &out) const;

  // Encrypt a tempfile, and outputs the result to another tempfile
  // will overwrite the outfile's content
  void Encrypt(const tempfile::TempFile &in,
               const tempfile::TempFile &out) const;

  // Encrypt a real file, and outputs the result to a tempfile
  // will overwrite the outfile content
  void Encrypt(std::ifstream &istream, const tempfile::TempFile &out) const;

  // Decrypt a tempfile, and outputs the result to another tempfile
  // will overwrite the outfile's content
  void Decrypt(const tempfile::TempFile &in,
               const tempfile::TempFile &out) const;

 private:
  byte key_[kKeySize];
};

}  // namespace symmetric
}  // namespace crypto
}  // namespace messageu

#endif