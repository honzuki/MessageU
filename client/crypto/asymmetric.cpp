#include "asymmetric.hpp"

#ifdef WIN32
#include <files.h>
#include <filters.h>
#include <modes.h>
#include <osrng.h>
#include <rsa.h>
#elif __linux__
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>
#endif

#include <fstream>
#include <tuple>

namespace messageu {
namespace crypto {
namespace asymmetric {

using CryptoPP::byte;

PublicKey::PublicKey(protocol::types::PublicKey public_key) {
  CryptoPP::StringSource ss(reinterpret_cast<const byte*>(public_key.data()),
                            public_key.size(), true);
  public_key_.Load(ss);
}

protocol::types::PublicKey PublicKey::Export() {
  protocol::types::PublicKey public_key{0};
  CryptoPP::ArraySink as(reinterpret_cast<byte*>(public_key.data()),
                         public_key.size());
  public_key_.Save(as);
  return public_key;
}

void PublicKey::Encrypt(const tempfile::TempFile& in,
                        const tempfile::TempFile& out) const {
  CryptoPP::AutoSeededRandomPool rng;
  CryptoPP::RSAES_OAEP_SHA_Encryptor e(public_key_);

  std::ifstream in_stream(in.path(), std::ios::binary);
  std::ofstream out_stream(out.path(), std::ios::binary);

  CryptoPP::FileSource{in_stream, true,
                       new CryptoPP::PK_EncryptorFilter{
                           rng, e, new CryptoPP::FileSink(out_stream)}};
}

PrivateKey::PrivateKey(const std::string& key) {
  CryptoPP::StringSource ss(key, true);
  private_key_.Load(ss);
}

std::string PrivateKey::Export() {
  std::string exported_key;
  CryptoPP::StringSink as(exported_key);
  private_key_.Save(as);
  return exported_key;
}

void PrivateKey::Decrypt(const tempfile::TempFile& in,
                         const tempfile::TempFile& out) const {
  static CryptoPP::AutoSeededRandomPool rng;
  CryptoPP::RSAES_OAEP_SHA_Decryptor d(private_key_);

  std::ifstream in_stream(in.path(), std::ios::binary);
  std::ofstream out_stream(out.path(), std::ios::binary);

  CryptoPP::FileSource{in_stream, true,
                       new CryptoPP::PK_DecryptorFilter{
                           rng, d, new CryptoPP::FileSink(out_stream)}};
}

std::tuple<PublicKey, PrivateKey> Generate() {
  CryptoPP::AutoSeededRandomPool rng;
  CryptoPP::RSA::PrivateKey private_key;
  private_key.Initialize(rng, kModulusbits);
  return std::make_tuple(PublicKey(private_key), PrivateKey(private_key));
}

}  // namespace asymmetric
}  // namespace crypto
}  // namespace messageu
