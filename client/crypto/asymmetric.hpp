// Masks the crypto library to provide an high-level abstraction for
// anything related to asymmetric encryption, and required by the client.

#ifndef CLIENT_CRYPTO_ASYMMETRIC_H
#define CLIENT_CRYPTO_ASYMMETRIC_H

#ifdef WIN32
#include <rsa.h>
#elif __linux__
#include <cryptopp/rsa.h>
#endif

#include <tuple>

#include "../protocol/types.hpp"

namespace messageu {
namespace crypto {
namespace asymmetric {
class PrivateKey;

constexpr unsigned int kModulusbits = 1024;

class PublicKey {
  friend std::tuple<PublicKey, PrivateKey> Generate();

 public:
  PublicKey(protocol::types::PublicKey public_key);
  protocol::types::PublicKey Export();

  // Encrypt a tempfile, and outputs the result to another tempfile
  // will overwrite the outfile content
  void Encrypt(const tempfile::TempFile &in,
               const tempfile::TempFile &out) const;

 private:
  PublicKey(const CryptoPP::RSA::PublicKey public_key)
      : public_key_(public_key) {}
  CryptoPP::RSA::PublicKey public_key_;
};

class PrivateKey {
  friend std::tuple<PublicKey, PrivateKey> Generate();

 public:
  PrivateKey(const std::string &key);
  std::string Export();

  // Decrypt a tempfile, and outputs the result to another tempfile
  // will overwrite the outfile content
  void Decrypt(const tempfile::TempFile &in,
               const tempfile::TempFile &out) const;

 private:
  PrivateKey(const CryptoPP::RSA::PrivateKey private_key)
      : private_key_(private_key) {}
  CryptoPP::RSA::PrivateKey private_key_;
};

// Generates a new pair of public & private key
std::tuple<PublicKey, PrivateKey> Generate();

}  // namespace asymmetric
}  // namespace crypto
}  // namespace messageu

#endif