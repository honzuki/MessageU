#include "symmetric.hpp"

#ifdef WIN32
#include <aes.h>
#include <files.h>
#include <filters.h>
#include <modes.h>
#include <osrng.h>
#elif __linux__
#include <cryptopp/aes.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#endif

#include <immintrin.h>  // _rdrand32_step

#include <fstream>

namespace messageu {
namespace crypto {
namespace symmetric {
namespace {
// size must be a 4 bytes align.
char* generate_key(char* buff, size_t size) {
  for (size_t i = 0; i < size; i += 4)
    _rdrand32_step(reinterpret_cast<unsigned int*>(&buff[i]));
  return buff;
}
}  // namespace

#ifdef WIN32
using CryptoPP::byte;
#endif

Key::Key() : key_{0} { generate_key(reinterpret_cast<char*>(key_), kKeySize); }

Key::Key(const char key[kKeySize]) { memcpy(key_, key, kKeySize); }

Key::Key(const Key& other) { memcpy(key_, other.key_, kKeySize); }

void Key::Export(const tempfile::TempFile& out) const {
  std::ofstream export_file(out.path());
  export_file.write(reinterpret_cast<const char*>(key_), kKeySize);
}

void Key::Encrypt(const tempfile::TempFile& in,
                  const tempfile::TempFile& out) const {
  std::ifstream in_stream(in.path(), std::ios::binary);
  Key::Encrypt(in_stream, out);
}

void Key::Encrypt(std::ifstream& istream, const tempfile::TempFile& out) const {
  byte iv[CryptoPP::AES::BLOCKSIZE]{0};  // unsafe but allowed for our purposes

  CryptoPP::AES::Encryption aesEncryption(key_, kKeySize);
  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption,
                                                              iv);

  std::ofstream out_stream(out.path(), std::ios::binary);
  CryptoPP::FileSource{istream, true,
                       new CryptoPP::StreamTransformationFilter{
                           cbcEncryption, new CryptoPP::FileSink(out_stream)}};
}

void Key::Decrypt(const tempfile::TempFile& in,
                  const tempfile::TempFile& out) const {
  byte iv[CryptoPP::AES::BLOCKSIZE]{0};  // unsafe but allowed for our purposes

  CryptoPP::AES::Decryption aesDecryption(key_, kKeySize);
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption,
                                                              iv);

  std::ifstream in_stream(in.path(), std::ios::binary);
  std::ofstream out_stream(out.path(), std::ios::binary);
  CryptoPP::FileSource{in_stream, true,
                       new CryptoPP::StreamTransformationFilter{
                           cbcDecryption, new CryptoPP::FileSink(out_stream)}};
}

}  // namespace symmetric
}  // namespace crypto
}  // namespace messageu
