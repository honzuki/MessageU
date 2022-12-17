#ifndef CLIENT_PROTOCOL_TYPES_H
#define CLIENT_PROTOCOL_TYPES_H

#include <array>
#include <cstddef>
#include <iosfwd>
#include <vector>

#include "../tempfile.hpp"

namespace messageu {
namespace protocol {
namespace types {

// The size of the block of data we'll
// read / write to the server.
constexpr size_t kBlockSize = 1024;

constexpr std::size_t kByteToBit = 8;

template <typename DATA_TYPE, std::size_t SIZE>
class LiteralType {
 public:
  using DataType = DATA_TYPE;
  LiteralType() = default;

  LiteralType(DataType value) : value_(value) {}

  // Parses the type from raw data,
  // the raw data is expected to be of size 'SIZE'
  LiteralType(unsigned char* raw_data) : value_(0) {
    // Convert to data type
    for (std::size_t i = 0; i < SIZE; ++i)  // little-endian to host
      value_ |= (static_cast<DataType>(raw_data[i]) << (i * kByteToBit));
  }

  // Serializes the Type back into raw data
  std::array<unsigned char, SIZE> Serialize() const {
    std::array<unsigned char, SIZE> buffer{0};
    for (std::size_t i = 0; i < SIZE; ++i)  // host to little-endian
      buffer[i] = (value_ >> (i * kByteToBit)) & 0xFF;
    return buffer;
  }

  DataType value() const { return value_; }

  operator std::string() const { return std::to_string(value_); }

  LiteralType& operator-=(const LiteralType& rhs) {
    value_ -= rhs.value_;
    return *this;
  }

  friend bool operator==(const LiteralType& lhs, const LiteralType& rhs) {
    return lhs.value_ == rhs.value_;
  }

  friend bool operator!=(const LiteralType& lhs, const LiteralType& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const LiteralType& lhs, const LiteralType& rhs) {
    return lhs.value_ < rhs.value_;
  }

 private:
  DataType value_;
};

constexpr std::size_t kVersionSize = 1;
using Version = LiteralType<std::uint8_t, kVersionSize>;

constexpr std::size_t kCodeSize = 2;
using Code = LiteralType<std::uint16_t, kCodeSize>;

constexpr std::size_t kPayloadSizeSize = 4;
using PayloadSize = LiteralType<std::uint32_t, kPayloadSizeSize>;

constexpr std::size_t kMessageIDSize = 4;
using MessageID = LiteralType<std::uint16_t, kMessageIDSize>;

constexpr std::size_t kMessageTypeSize = 1;
using MessageType = LiteralType<std::uint8_t, kMessageTypeSize>;
namespace MessageTypes {
constexpr MessageType::DataType SymmetricKeyRequest = 1, SymmetricKey = 2,
                                TextMessage = 3, File = 4;
}  // namespace MessageTypes

constexpr std::size_t kContentSizeSize = 4;
using ContentSize = LiteralType<std::uint32_t, kContentSizeSize>;

constexpr std::size_t kClientIDSize = 16;
using ClientID = std::array<unsigned char, kClientIDSize>;

constexpr std::size_t kPublicKeySize = 160;
using PublicKey = std::array<unsigned char, kPublicKeySize>;

constexpr std::size_t kUsernameSize = 255;
using Username = std::array<unsigned char, kUsernameSize>;

// We are not allowed to use smart pointer
class Content {
 public:
  // Name the dump_file.
  // A good name can be the message_id, if exists.
  Content(const std::string& filename);
  Content(Content& other);
  ~Content();

  // Get the tempfile.
  // generally, use this to read & write
  const tempfile::TempFile& operator*() const;
  const tempfile::TempFile* operator->() const;

 private:
  std::size_t* reference_count_;

  // The content have a dynamic size, so
  // we save it in a temporary file.
  tempfile::TempFile* dump_file_;
};

}  // namespace types
}  // namespace protocol
}  // namespace messageu

#endif