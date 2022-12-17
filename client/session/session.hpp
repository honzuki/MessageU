#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <boost/asio.hpp>
#include <filesystem>
#include <map>
#include <string>

#include "../config.hpp"
#include "../crypto/asymmetric.hpp"
#include "../crypto/symmetric.hpp"
#include "../protocol/request.hpp"
#include "../protocol/response.hpp"
#include "../protocol/types.hpp"
#include "types.hpp"

namespace messageu {
namespace session {

class Session {
  // This structure is used to mask all communication with the server,
  // and serves as an abstraction layer for the low-level implementaiton
  // of the protocol.
  // Be aware that any function that communicate with the server may not
  // catch a low-level exception, without explicitly declaring so!
  //
  // Any function that connects to the server may throw std::runtime_error,
  // if it can not initialize the connection.
  //
  // Any function that declares it self as "[Authorized]"" requires
  // you to be registered to the server, otherwise it will throw
  // a session::exceptions::UnauthorizedRequest exception.
  //
  // Any function that requires a target username will try to resolve the
  // target by its username, and throws session::exceptions::UnknownTarget
  // if it fails to do so.
 public:
  Session(const config::ServerInfo &server_info) : server_info_(server_info) {}

  // Will try to read the info from the given info file,
  // but will not complain if it fails
  Session(const config::ServerInfo &server_info,
          std::filesystem::path info_file);

  // Registers the client to the server
  // one should make sure to register before using any other function.
  //
  // Saves the client info to 'info_file'
  //
  // Throws:
  //  session::exceptions::AlreadyRegistered: you're already
  //    registered to the server.
  //  session::exceptions::UsernameTooLong: the username is too long.
  void Register(std::string username, std::filesystem::path info_file);

  // [Authorized]
  // Polls the client list from the server.
  // For each client calls the callback with the client username.
  //
  // You can use a client username returned from this function
  // with any of the other functions.
  void UpdateClientList(
      std::function<void(const std::string &username)> callback);

  // [Authorized]
  // Polls a target public's key from the server, by username.
  void GetPublicKey(const std::string &target_username);

  // [Authorized]
  // Polls pending messages from the server.
  // The function takes care of the decryption procedure,
  // and calls the callback with a wrapper, easy-to-use, class.
  //
  // When you receive a symmetric key, the session takes care of saving it,
  // even if you never requested it. However, when you get a request for key,
  // you have to decide if you actually want to send it.
  void RetrievePendingMessages(
      std::function<void(const types::Message &message)> callback);

  // [Authorized]
  // Sends a text message
  //
  // Throws:
  //    session::exceptions::MissingKey: does not have a symmetric key
  //        for the target
  void SendMessage(const std::string &target_username, const std::string &text);

  // [Authorized]
  // Sends a file
  //
  // Throws:
  //    session::exceptions::MissingKey: does not have a symmetric key
  //        for the target
  //    session::exceptions::UnknownFilePath can't open the file
  void SendFile(const std::string &target_username,
                const std::filesystem::path &file);

  // [Authorized]
  // Request a client to send you a symmertic key
  void RequestSymmetricKey(const std::string &target_username);

  // [Authorized]
  // Generates a new symmertic key, and sends it to the client.
  //
  // Be aware! even if there was a symmetric key already,
  // it'll generate a new one, and overrite the old one.
  //
  // Throws:
  //    session::exceptions::MissingKey: does not have the target's public key
  void SendSymmetricKey(const std::string &target_username);

  ~Session();

  // Conceptually, session is too specific to be copyable
  Session(Session &) = delete;

 private:
  // Internal function that handles all the boiler-plate related to
  // initializing a new connection with the server
  boost::asio::ip::tcp::socket OpenConnection(
      const protocol::request::Header &request);

  // Internal function that tries to resolve a client by its username,
  // and throws session::exceptions::UnknownTarget if it could not
  // find the target.
  types::Client &ResolveTarget(const std::string &username);

  // Internal function that tries to resolve a client by its id,
  // and throws session::exceptions::UnknownTarget if it could not
  // find the target.
  types::Client &ResolveTarget(const protocol::types::ClientID &id);

  // Internal function that helps decryting a symmetric key,
  // using our own private key
  //
  // Throws std::runtime_error if fails to decrypt the key
  crypto::symmetric::Key DecryptSymmetricKey(
      const tempfile::TempFile &encrypted_key_dump);

  config::ServerInfo server_info_;
  config::MyInfo *my_info_ = nullptr;

  // maps ids/username [they're unique] to clients.
  // the idea to use map both ways is to take advantage of the hash_table,
  // and avoid wasteful searches.
  std::map<std::string, types::Client *> username_to_client_;
  std::map<protocol::types::ClientID, types::Client *> id_to_client_;
};

}  // namespace session
}  // namespace messageu

#endif