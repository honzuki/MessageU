#include "exceptions.hpp"

#include <string>

namespace messageu {
namespace protocol {
namespace exceptions {
GeneralException::GeneralException(const std::string &reason)
    : std::runtime_error(reason) {}

UnexpectedReponseError::UnexpectedReponseError(const types::Code &expected_code,
                                               const types::Code &received_code)
    : GeneralException("Expected response code " + std::string(expected_code) +
                       ", but received " + std::string(received_code)) {}

PayloadMismatch::PayloadMismatch(const types::PayloadSize &expected_size,
                                 const types::PayloadSize &received_size)
    : GeneralException(
          "Expected payload of size " + std::string(expected_size) +
          ", but received payload of size" + std::string(received_size)) {}

ContentMismatch::ContentMismatch()
    : GeneralException("The content size does not match the payload size") {}

ContentSizeLimit::ContentSizeLimit(std::uintmax_t max_size_bytes,
                                   std::uintmax_t received_size_bytes)
    : GeneralException("Tried to send content of size " +
                       std::to_string(received_size_bytes) +
                       "bytes, but the limit is " +
                       std::to_string(max_size_bytes) + "bytes") {}

GeneralError::GeneralError()
    : GeneralException("Server responded with an error") {}
}  // namespace exceptions
}  // namespace protocol
}  // namespace messageu