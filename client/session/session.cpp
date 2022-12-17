#include "session.hpp"

#include <fstream>

#include "exceptions.hpp"

namespace messageu {
namespace session {

Session::Session(const config::ServerInfo &server_info,
                 std::filesystem::path info_file)
    : server_info_(server_info) {
  try {
    my_info_ = new config::MyInfo(info_file);
  } catch (const std::invalid_argument &) {
    my_info_ = nullptr;
  }
}

void Session::Register(std::string username, std::filesystem::path info_file) {
  if (my_info_) throw session::exceptions::AlreadyRegistered();
  if (username.size() > protocol::types::kUsernameSize)
    throw session::exceptions::UsernameTooLong(username);

  // Generate keys
  auto [public_key, private_key] = crypto::asymmetric::Generate();

  // Register with the server
  protocol::types::Username raw_username{0};
  std::copy(username.begin(), username.end(), raw_username.begin());
  auto socket = OpenConnection(
      protocol::request::Register(raw_username, public_key.Export()));

  // Hopefully, register succeeded, you can save the info.
  auto response = protocol::response::Register(socket);
  my_info_ = new config::MyInfo(username, response.client_id, private_key);
  my_info_->Save(info_file);
}

void Session::UpdateClientList(
    std::function<void(const std::string &username)> callback) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();

  // Reset old tables
  for (auto &[username, client_ptr] : username_to_client_) {
    delete client_ptr;
  }
  username_to_client_.clear();
  id_to_client_.clear();

  // A complex response that needs an ownership over the socket
  auto response = protocol::response::ClientList(
      OpenConnection(protocol::request::ClientList(my_info_->client_id())));
  response.ReadClients([&](protocol::response::Client raw_client) {
    std::string parsed_name(std::begin(raw_client.name),
                            std::end(raw_client.name));
    // remove all dead characters, this is necessary for map
    parsed_name = parsed_name.substr(0, parsed_name.find('\0'));
    auto *client = new types::Client(raw_client.id, parsed_name);
    username_to_client_[parsed_name] = client;
    id_to_client_[raw_client.id] = client;
    callback(parsed_name);
  });
}

void Session::GetPublicKey(const std::string &target_username) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();

  auto &target = ResolveTarget(target_username);
  auto socket = OpenConnection(
      protocol::request::GetPublicKey(my_info_->client_id(), target.id()));
  auto response = protocol::response::PublicKey(socket);
  target.set_public_key(response.target_public_key);
}

void Session::RetrievePendingMessages(
    std::function<void(const types::Message &message)> callback) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();

  // A complex response that needs an ownership over the socket
  auto response = protocol::response::PendingMessages(OpenConnection(
      protocol::request::RetrievePendingMessages(my_info_->client_id())));
  response.ReadMessages([&](protocol::response::Message &message) {
    types::Client *sender;
    try {
      sender = id_to_client_.at(message.sender_id);
    } catch (const std::out_of_range &) {
      callback(
          types::ErrorMessage("Unknown", "Can not resolve the sender id."));
      return;
    }
    try {
      switch (message.type.value()) {
        using namespace protocol::types;
        case MessageTypes::SymmetricKeyRequest:
          callback(types::SymmetricKeyRequestMessage(sender->username()));
          break;
        case MessageTypes::SymmetricKey:
          sender->set_symmetric_key(DecryptSymmetricKey(*message.content()));
          callback(types::ReceivedSymmetricKeyMessage(sender->username()));
          break;
        case MessageTypes::File: {
          auto res_msg = types::FileMessage(
              sender->username(),
              new tempfile::TempFile(
                  message.content()->path().filename().string() + ".decrypted",
                  /*auto_delete=*/false));
          sender->symmetric_key().Decrypt(*message.content(),
                                          *res_msg.dump_file_);
          callback(res_msg);
        } break;
        case MessageTypes::TextMessage: {
          auto res_msg = types::TextMessage(
              sender->username(),
              new tempfile::TempFile(
                  message.content()->path().filename().string() +
                  ".decrypted"));
          sender->symmetric_key().Decrypt(*message.content(),
                                          *res_msg.dump_file_);
          callback(res_msg);
        } break;
        default:
          // We can't decrypt it...
          callback(types::EncryptedMessage(sender->username()));
      }
    } catch (const CryptoPP::Exception &) {
      callback(types::EncryptedMessage(sender->username()));
    } catch (const exceptions::MissingKey &) {
      callback(types::EncryptedMessage(sender->username()));
    } catch (const std::runtime_error &e) {
      callback(types::ErrorMessage(sender->username(), e.what()));
    }
  });
}

void Session::SendMessage(const std::string &target_username,
                          const std::string &text) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();
  auto &target = ResolveTarget(target_username);

  // Write the message to a temp file
  auto temp_content_file = tempfile::TempFile("new_message.decrypt");
  std::ofstream content_file(temp_content_file.path());
  content_file << text;
  content_file.close();

  // Encrypt the message
  auto content = protocol::types::Content("new_message");
  target.symmetric_key().Encrypt(temp_content_file, *content);

  // Send to server
  auto socket = OpenConnection(protocol::request::SendMessage(
      my_info_->client_id(), target.id(),
      protocol::types::MessageTypes::TextMessage, content));
  protocol::response::MessageSent{socket};  // do nothing...
}

void Session::SendFile(const std::string &target_username,
                       const std::filesystem::path &file) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();
  auto &target = ResolveTarget(target_username);
  std::ifstream content_file(file);
  if (content_file.fail()) throw exceptions::UnknownFilePath(file.string());

  // Encrypt message
  auto content = protocol::types::Content("new_file");
  target.symmetric_key().Encrypt(content_file, *content);

  // Send to server
  auto socket = OpenConnection(protocol::request::SendMessage(
      my_info_->client_id(), target.id(), protocol::types::MessageTypes::File,
      content));
  protocol::response::MessageSent{socket};  // do nothing...
}

void Session::RequestSymmetricKey(const std::string &target_username) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();
  auto &target = ResolveTarget(target_username);

  // This is an empty file, as we don't send,
  // any additional data in such a request.
  auto content = protocol::types::Content("empty_file");

  // Send to server
  auto socket = OpenConnection(protocol::request::SendMessage(
      my_info_->client_id(), target.id(),
      protocol::types::MessageTypes::SymmetricKeyRequest, content));
  protocol::response::MessageSent{socket};  // do nothing...
}

void Session::SendSymmetricKey(const std::string &target_username) {
  if (!my_info_) throw session::exceptions::UnauthorizedRequest();
  auto &target = ResolveTarget(target_username);

  // Generate&Save the new symmetric key
  auto dump_key = tempfile::TempFile("symmetric_key");
  target.set_symmetric_key(crypto::symmetric::Key());
  target.symmetric_key().Export(dump_key);

  // Encrypt the key
  auto content = protocol::types::Content("symmetric_key.encrypted");
  target.public_key().Encrypt(dump_key, *content);

  // Send to server
  auto socket = OpenConnection(protocol::request::SendMessage(
      my_info_->client_id(), target.id(),
      protocol::types::MessageTypes::SymmetricKey, content));
  protocol::response::MessageSent{socket};  // do nothing...
}

boost::asio::ip::tcp::socket Session::OpenConnection(
    const protocol::request::Header &request) {
  try {
    static boost::asio::io_context io_context;
    static boost::asio::ip::tcp::resolver resolver(io_context);
    static auto server_address =
        resolver.resolve(server_info_.ip(), server_info_.port());

    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::connect(socket, server_address);
    request.send(socket);  // send the response
    return socket;
  } catch (const boost::exception &) {
    throw std::runtime_error("can not initialize a connection with the server");
  }
}

types::Client &Session::ResolveTarget(const std::string &username) {
  try {
    return *(username_to_client_.at(username));
  } catch (const std::out_of_range &) {
    throw session::exceptions::UnknownTarget(username);
  }
}

crypto::symmetric::Key Session::DecryptSymmetricKey(
    const tempfile::TempFile &encrypted_key_dump) {
  tempfile::TempFile decrypted_key_dump(
      encrypted_key_dump.path().filename().string() + ".decrypted");
  try {
    my_info_->private_key().Decrypt(encrypted_key_dump, decrypted_key_dump);

    std::ifstream dump_key(decrypted_key_dump.path());
    if (dump_key.fail()) throw std::runtime_error("could not decrypt the key");
    char raw_key[crypto::symmetric::kKeySize]{0};
    dump_key.read(raw_key, crypto::symmetric::kKeySize);
    if (dump_key.gcount() != crypto::symmetric::kKeySize)
      throw std::runtime_error("key is too long");

    return raw_key;
  } catch (const CryptoPP::Exception &) {
    throw std::runtime_error("could not decrypt the key");
  }
}

Session::~Session() {
  delete my_info_;

  // the maps are sharing their pointers.
  for (auto &[username, client_ptr] : username_to_client_) delete client_ptr;
}

}  // namespace session
}  // namespace messageu