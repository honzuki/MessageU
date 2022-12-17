// Bundles all the exceptions related to an active session.

#ifndef CLIENT_SESSION_EXCEPTIONS_H
#define CLIENT_SESSION_EXCEPTIONS_H

#include <stdexcept>

#include "../protocol/types.hpp"

namespace messageu {
namespace session {
namespace exceptions {

class GeneralError : public std::runtime_error {
 public:
  GeneralError(const std::string &reason);
};

// Missing target key
class MissingKey : public GeneralError {
 public:
  MissingKey(const std::string &key_type);
};

// The given username is too long
class UsernameTooLong : public GeneralError {
 public:
  UsernameTooLong(const std::string &username);
};

// Can't register more than 1 time
class AlreadyRegistered : public GeneralError {
 public:
  AlreadyRegistered();
};

// Tried to perform a request that requires authorization
// without registring with the server
class UnauthorizedRequest : public GeneralError {
 public:
  UnauthorizedRequest();
};

// Can't find the target (of a request) in the internal structure
class UnknownTarget : public GeneralError {
 public:
  UnknownTarget(const std::string &target_username);
};

// Can't find the file path
class UnknownFilePath : public GeneralError {
 public:
  UnknownFilePath(const std::string &path);
};

}  // namespace exceptions
}  // namespace session
}  // namespace messageu

#endif