#include "client.hpp"

#include "config.hpp"
#include "protocol/exceptions.hpp"
#include "session/exceptions.hpp"
#include "session/session.hpp"
#include "ui.hpp"

namespace messageu {

namespace {
// boilerplate code
void mask_request(std::ostream& output, std::function<void()> callback) {
  try {
    callback();
  } catch (const session::exceptions::GeneralError& e) {
    output << e.what();
  } catch (const protocol::exceptions::GeneralException& e) {
    output << e.what();
  }
}
}  // namespace

void start_client() {
  session::Session client_session(config::ServerInfo(kServerInfoFile),
                                  kClientInfoFile);
  ui::UI ui("MessageU client at your service");
  std::function<void(std::ostream&)> callback;

  // Register
  callback = [&](std::ostream& ostream) {
    std::string username = ui.ReadLine("Enter username: ");
    mask_request(ostream, [&]() {
      client_session.Register(username, kClientInfoFile);
      ostream << "Registered successfully!";
    });
  };
  ui.RegisterCmd(kRegisterCode, kRegisterTitle, callback);

  // Poll memberlist
  callback = [&](std::ostream& ostream) {
    mask_request(ostream, [&]() {
      size_t client_count = 0;
      client_session.UpdateClientList([&](const std::string& username) {
        ostream << ++client_count << ". \"" << username << "\"\n";
      });
      if (!client_count) ostream << "There are no other registered clients";
    });
  };
  ui.RegisterCmd(kMemberListCode, kMemberListTitle, callback);

  // Request public key
  callback = [&](std::ostream& ostream) {
    std::string target_username = ui.ReadLine("Enter the target's username: ");
    mask_request(ostream, [&]() {
      client_session.GetPublicKey(target_username);
      ostream << "Received " << target_username
              << "\'s public key, successfully!";
    });
  };
  ui.RegisterCmd(kRequestPublicKeyCode, kRequestPublicKeyTitle, callback);

  // Poll pending messages
  callback = [&](std::ostream& ostream) {
    mask_request(ostream, [&]() {
      size_t message_count = 0;
      client_session.RetrievePendingMessages(
          [&](const session::types::Message& message) {
            ostream << message;
            ++message_count;
          });
      if (!message_count) ostream << "There are no pending messages";
    });
  };
  ui.RegisterCmd(kRetreivePendingMessagesCode, kRetreivePendingMessagesTitle,
                 callback);

  // Send text message
  callback = [&](std::ostream& ostream) {
    std::string target_username = ui.ReadLine("Enter the target's username: ");
    std::string message = ui.ReadLine("Enter your message: ");
    mask_request(ostream, [&]() {
      client_session.SendMessage(target_username, message);
      ostream << "The message has been sent successfully";
    });
  };
  ui.RegisterCmd(kSendTextMessageCode, kSendTextMessageTitle, callback);

  // Request symmetric key
  callback = [&](std::ostream& ostream) {
    std::string target_username = ui.ReadLine("Enter the target's username: ");
    mask_request(ostream, [&]() {
      client_session.RequestSymmetricKey(target_username);
      ostream << "Request for " << target_username
              << "\'s symmetric key has been sent successfully";
    });
  };
  ui.RegisterCmd(kRequestSymmerticKeyCode, kRequestSymmerticKeyTitle, callback);

  // Send symmetric key
  callback = [&](std::ostream& ostream) {
    std::string target_username = ui.ReadLine("Enter the target's username: ");
    mask_request(ostream, [&]() {
      client_session.SendSymmetricKey(target_username);
      ostream << "Symmetric key has been sent successfully";
    });
  };
  ui.RegisterCmd(kSendSymmetricKeyCode, kSendSymmetricKeyTitle, callback);

  // Send file
  callback = [&](std::ostream& ostream) {
    std::string target_username = ui.ReadLine("Enter the target's username: ");
    std::string file_path =
        ui.ReadLine("Enter file path (relative to client or absolute): ");
    mask_request(ostream, [&]() {
      client_session.SendFile(target_username, file_path);
      ostream << "The file has been sent successfully";
    });
  };
  ui.RegisterCmd(kSendFileCode, kSendFileTitle, callback);

  ui.Run(kExitCode);
}

}  // namespace messageu
