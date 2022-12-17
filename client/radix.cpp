#include "radix.hpp"

#ifdef WIN32
#include <base64.h>
#elif __linux__
#include <cryptopp/base64.h>
#endif

#include <cmath>
#include <iomanip>
#include <sstream>

namespace messageu {
namespace radix {
namespace hex {
std::string Encode(unsigned char* bytes, std::size_t len) {
  std::stringstream hex;
  hex << std::hex;
  for (std::size_t i = 0; i < len; ++i)
    hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);

  return hex.str();
}

std::vector<unsigned char> Decode(std::string hex) {
  std::vector<unsigned char> bytes;
  for (std::size_t i = 0; i < hex.size(); i += 2)
    bytes.push_back(
        static_cast<unsigned char>(std::stoul(hex.substr(i, 2), nullptr, 16)));

  return bytes;
}
}  // namespace hex

namespace base64 {
std::string Encode(std::string decoded) {
  // source - https://cryptopp.com/wiki/Base64Encoder
  std::string encoded;
  CryptoPP::StringSource ss(
      decoded, true,
      new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded),
                                  /*insertLineBreaks=*/false)  // Base64Encoder
  );                                                           // StringSource

  return encoded;
}

std::string Decode(std::string encoded) {
  // source - https://cryptopp.com/wiki/Base64Decoder
  std::string decoded;
  CryptoPP::StringSource ss(
      encoded, true,
      new CryptoPP::Base64Decoder(
          new CryptoPP::StringSink(decoded))  // Base64Decoder
  );                                          // StringSource
  return decoded;
}
}  // namespace base64

}  // namespace radix
}  // namespace messageu