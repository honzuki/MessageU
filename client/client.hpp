#ifndef CLIENT_H
#define CLIENT_H

#include <cinttypes>
#include <cstddef>
#include <iosfwd>

namespace messageu {

constexpr char kServerInfoFile[] = "server.info";
constexpr char kClientInfoFile[] = "my.info";

constexpr std::size_t kExitCode = 0;

constexpr std::size_t kRegisterCode = 110;
constexpr char kRegisterTitle[] = "Register";

constexpr std::size_t kMemberListCode = 120;
constexpr char kMemberListTitle[] = "Request for clients list";

constexpr std::size_t kRequestPublicKeyCode = 130;
constexpr char kRequestPublicKeyTitle[] = "Request for public key";

constexpr std::size_t kRetreivePendingMessagesCode = 140;
constexpr char kRetreivePendingMessagesTitle[] = "Request for waiting messages";

constexpr std::size_t kSendTextMessageCode = 150;
constexpr char kSendTextMessageTitle[] = "Send a text message";

constexpr std::size_t kRequestSymmerticKeyCode = 151;
constexpr char kRequestSymmerticKeyTitle[] = "Send a request for symmetric key";

constexpr std::size_t kSendSymmetricKeyCode = 152;
constexpr char kSendSymmetricKeyTitle[] = "Send your symmetric key";

constexpr std::size_t kSendFileCode = 153;
constexpr char kSendFileTitle[] = "Send a file";

// Starts the client
void start_client();

}  // namespace messageu

#endif