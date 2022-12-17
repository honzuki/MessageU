// Utilities related to the different numerical bases

#ifndef CLIENT_RADIX_H
#define CLIENT_RADIX_H

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace messageu {
namespace radix {

namespace hex {
std::string Encode(unsigned char* bytes, std::size_t len);
std::vector<unsigned char> Decode(std::string hex);
}  // namespace hex

namespace base64 {
std::string Encode(std::string decoded);
std::string Decode(std::string encoded);
}  // namespace base64

}  // namespace radix
}  // namespace messageu

#endif