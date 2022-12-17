#include "exceptions.hpp"

namespace messageu {
namespace session {
namespace exceptions {
GeneralError::GeneralError(const std::string &reason)
    : std::runtime_error(reason) {}

MissingKey::MissingKey(const std::string &key_type)
    : GeneralError("Missing the " + key_type + " key of the target") {}

UsernameTooLong::UsernameTooLong(const std::string &username)
    : GeneralError("The username " + username + " is too long") {}

AlreadyRegistered::AlreadyRegistered()
    : GeneralError("Already registered to the system") {}

UnauthorizedRequest::UnauthorizedRequest()
    : GeneralError(
          "Can not perform this action without registering to the server") {}

UnknownTarget::UnknownTarget(const std::string &target_username)
    : GeneralError("Could not resolve the username: \"" + target_username +
                   "\"") {}

UnknownFilePath::UnknownFilePath(const std::string &path)
    : GeneralError("Failed to open " + path) {}

}  // namespace exceptions
}  // namespace session
}  // namespace messageu