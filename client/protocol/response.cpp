#include "response.hpp"

#include <fstream>

#include "exceptions.hpp"
#include "types.hpp"

namespace messageu {
namespace protocol {
namespace response {

namespace {
// Reads from the socket exactly 'size' bytes
void read_all(boost::asio::ip::tcp::socket &socket, unsigned char *data,
              size_t count) {
  size_t read_amount = 0;
  while (read_amount < count) {
    read_amount += boost::asio::read(
        socket, boost::asio::buffer(data + read_amount, count - read_amount));
  }
}

}  // namespace

Header::Header(const types::Code &expected_code,
               boost::asio::ip::tcp::socket &socket) {
  constexpr auto header_size =
      types::kVersionSize + types::kCodeSize + types::kPayloadSizeSize;
  unsigned char data[header_size];
  read_all(socket, data, sizeof(data));
  server_version_ = data;
  code_ = data + types::kVersionSize;
  payload_size_ = data + types::kVersionSize + types::kCodeSize;

  if (code_ == kGeneralError) throw exceptions::GeneralError();
  if (code_ != expected_code)
    throw exceptions::UnexpectedReponseError(expected_code, code_);
}

Register::Register(boost::asio::ip::tcp::socket &socket)
    : Header(kRegisterCode, socket) {
  constexpr auto payload_size = types::kClientIDSize;
  if (payload_size != payload_size_)
    throw exceptions::PayloadMismatch(payload_size, payload_size_);
  read_all(socket, client_id.data(), types::kClientIDSize);
}

ClientList::ClientList(boost::asio::ip::tcp::socket &&socket)
    : Header(kClientListCode, socket), socket_(std::move(socket)) {
  constexpr auto client_size = types::kClientIDSize + types::kUsernameSize;
  client_count_ = payload_size_.value() / client_size;
}

void ClientList::ReadClients(
    std::function<void(Client &client)> proccess_client) {
  while ((client_count_--)) {
    Client client;
    read_all(socket_, client.id.data(), types::kClientIDSize);
    read_all(socket_, client.name.data(), types::kUsernameSize);

    proccess_client(client);
  }
  client_count_ = 0;
}

PublicKey::PublicKey(boost::asio::ip::tcp::socket &socket)
    : Header(kPublicKeyCode, socket) {
  constexpr auto payload_size = types::kClientIDSize + types::kPublicKeySize;
  if (payload_size != payload_size_)
    throw exceptions::PayloadMismatch(payload_size, payload_size_);
  read_all(socket, target_id.data(), types::kClientIDSize);
  read_all(socket, target_public_key.data(), types::kPublicKeySize);
}

MessageSent::MessageSent(boost::asio::ip::tcp::socket &socket)
    : Header(kMessageSentCode, socket) {
  constexpr auto payload_size = types::kClientIDSize + types::kMessageIDSize;
  if (payload_size != payload_size_)
    throw exceptions::PayloadMismatch(payload_size, payload_size_);
  read_all(socket, target_id.data(), types::kClientIDSize);

  unsigned char data[types::kMessageIDSize];
  read_all(socket, data, sizeof(data));
  message_id = data;
}

Message::Message(Message &&other) : content_(other.content_) {
  other.content_ = nullptr;
}

types::Content Message::content() {
  if (!content_) throw std::runtime_error("message content does not exist");
  return *content_;
}

// Deletes old content if exist.
void Message::CreateContent(const std::string &filename) {
  if (content_) delete content_;
  content_ = new types::Content(filename);
}

PendingMessages::PendingMessages(boost::asio::ip::tcp::socket &&socket)
    : Header(kPendingMessagesCode, socket), socket_(std::move(socket)) {}

void PendingMessages::ReadMessages(
    std::function<void(Message &message)> proccess_message) {
  constexpr auto message_header_size =
      types::kClientIDSize + types::kMessageIDSize + types::kMessageTypeSize +
      types::kContentSizeSize;

  while (payload_size_.value() >= message_header_size) {
    payload_size_ -= message_header_size;

    Message message;
    read_all(socket_, message.sender_id.data(), types::kClientIDSize);

    // Read literal values
    unsigned char data[types::kMessageIDSize + types::kMessageTypeSize +
                       types::kContentSizeSize];
    read_all(socket_, data, sizeof(data));
    message.id = data;
    message.type = data + types::kMessageIDSize;
    types::ContentSize content_size(data + types::kMessageIDSize +
                                    types::kMessageTypeSize);

    // Read content if exists
    if (content_size.value()) {
      if (payload_size_.value() < content_size.value())
        throw exceptions::ContentMismatch();
      payload_size_ -= content_size;

      using SizeT = types::ContentSize::DataType;
      message.CreateContent(
          "message_" + std::to_string(message.id.value()));  // message_{id}
      std::ofstream content_file(message.content()->path(),
                                 std::ofstream::binary);
      SizeT read_size{0};
      for (SizeT read = 0; read < content_size.value(); read += read_size) {
        char data[types::kBlockSize]{0};
        auto to_read_size = content_size.value() - read;
        if (to_read_size > types::kBlockSize)  // limit the amount you read
          to_read_size = types::kBlockSize;
        read_size =
            boost::asio::read(socket_, boost::asio::buffer(data, to_read_size));
        // write as much as you actually read
        content_file.write(data, read_size);
      }
    }

    proccess_message(message);
  }
}

}  // namespace response
}  // namespace protocol
}  // namespace messageu