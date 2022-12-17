#include "request.hpp"

#include <cmath>
#include <fstream>

#include "exceptions.hpp"

namespace messageu {
namespace protocol {
namespace request {

Header::Header(const types::ClientID &sender_id, const types::Code &code,
               const types::PayloadSize &payload_size)
    : sender_id_(sender_id), code_(code), payload_size_(payload_size) {}

void Header::send(boost::asio::ip::tcp::socket &socket) const {
  boost::asio::write(socket, boost::asio::buffer(sender_id_));
  boost::asio::write(socket, boost::asio::buffer(kClientVersion.Serialize()));
  boost::asio::write(socket, boost::asio::buffer(code_.Serialize()));
  boost::asio::write(socket, boost::asio::buffer(payload_size_.Serialize()));
}

types::ClientID Register::dump_id_;

Register::Register(const types::Username &username,
                   const types::PublicKey &public_key)
    : Header(dump_id_, kRegisterCode,
             types::kUsernameSize + types::kPublicKeySize),
      username_(username),
      public_key_(public_key) {}

void Register::send(boost::asio::ip::tcp::socket &socket) const {
  Header::send(socket);
  boost::asio::write(socket, boost::asio::buffer(username_));
  boost::asio::write(socket, boost::asio::buffer(public_key_));
}

ClientList::ClientList(const types::ClientID &sender_id)
    : Header(sender_id, kClientListCode,
             static_cast<types::PayloadSize::DataType>(0)) {}

void ClientList::send(boost::asio::ip::tcp::socket &socket) const {
  Header::send(socket);
}

GetPublicKey::GetPublicKey(const types::ClientID &sender_id,
                           const types::ClientID &target_id)
    : Header(sender_id, kPublicKeyCode, types::kClientIDSize),
      target_id_(target_id) {}

void GetPublicKey::send(boost::asio::ip::tcp::socket &socket) const {
  Header::send(socket);
  boost::asio::write(socket, boost::asio::buffer(target_id_));
}

SendMessage::SendMessage(const types::ClientID &sender_id,
                         const types::ClientID &target_id,
                         const types::MessageType &type, types::Content content)
    : Header(sender_id, kSendMessagesCode,
             types::kClientIDSize + types::kMessageTypeSize +
                 types::kContentSizeSize + content->size()),
      target_id_(target_id),
      type_(type),
      content_(content) {}

void SendMessage::send(boost::asio::ip::tcp::socket &socket) const {
  static const std::uintmax_t max_content_size =
      std::pow(2, types::kContentSizeSize * 8) - 1;
  // we don't care about an inconsistency with the payload size,
  // that's the job of the server... we only need to avoid overflow.
  if (content_->size() > max_content_size)
    throw exceptions::ContentSizeLimit(max_content_size, content_->size());
  Header::send(socket);
  boost::asio::write(socket, boost::asio::buffer(target_id_));
  boost::asio::write(socket, boost::asio::buffer(type_.Serialize()));
  boost::asio::write(
      socket,
      boost::asio::buffer(types::ContentSize(content_->size()).Serialize()));

  std::ifstream content_file(content_->path(), std::ofstream::binary);
  while (content_file) {
    char data[types::kBlockSize]{0};
    content_file.read(data, types::kBlockSize);
    if (content_file.gcount())  // write as much as you actually read
      boost::asio::write(socket,
                         boost::asio::buffer(data, content_file.gcount()));
  }
}

RetrievePendingMessages::RetrievePendingMessages(
    const types::ClientID &sender_id)
    : Header(sender_id, kRetrievePendingMessageCode,
             static_cast<types::PayloadSize::DataType>(0)) {}

void RetrievePendingMessages::send(boost::asio::ip::tcp::socket &socket) const {
  Header::send(socket);
}

}  // namespace request
}  // namespace protocol
}  // namespace messageu