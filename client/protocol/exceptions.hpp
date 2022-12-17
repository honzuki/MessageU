#ifndef CLIENT_PROTOCOL_EXCEPTIONS_H
#define CLIENT_PROTOCOL_EXCEPTIONS_H

#include <cstddef>
#include <iosfwd>
#include <stdexcept>

#include "types.hpp"

namespace messageu {
namespace protocol {
namespace exceptions {
class GeneralException : public std::runtime_error {
 public:
  GeneralException(const std::string &reason);
};

// Received an unexpected response from the server
class UnexpectedReponseError : public GeneralException {
 public:
  UnexpectedReponseError(const types::Code &expected_code,
                         const types::Code &received_code);
};

// A mismatch between the size of the payload received
// from the server, and the size the client expected
class PayloadMismatch : public GeneralException {
 public:
  PayloadMismatch(const types::PayloadSize &expected_size,
                  const types::PayloadSize &received_size);
};

// A mismatch between the size of the payload size and the content size
class ContentMismatch : public GeneralException {
 public:
  ContentMismatch();
};

// The size of the content is too big
class ContentSizeLimit : public GeneralException {
 public:
  ContentSizeLimit(std::uintmax_t max_size_bytes,
                   std::uintmax_t received_size_bytes);
};

// Received general error from the server.
class GeneralError : public GeneralException {
 public:
  GeneralError();
};

}  // namespace exceptions
}  // namespace protocol
}  // namespace messageu

#endif