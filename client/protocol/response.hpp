#ifndef CLIENT_PROTOCOL_RESPONSE_H
#define CLIENT_PROTOCOL_RESPONSE_H

#include <boost/asio.hpp>

#include "types.hpp"

namespace messageu {
namespace protocol {
namespace response {

constexpr types::Code::DataType kRegisterCode = 2100, kClientListCode = 2101,
                                kPublicKeyCode = 2102, kMessageSentCode = 2103,
                                kPendingMessagesCode = 2104,
                                kGeneralError = 9000;

// The constructor of each of the response types
// deserializes the response directely from an open socket.
// It may throw:
//  protocol::exceptions::UnexpectedReponseError:
//      The response received from the server doesn't match
//      the structure you're trying to initialize.
//  protocol::exceptions::PayloadMismatch:
//      There is a mismatch between the size of the payload
//      received from the user and the size of the structure
//      the client knows.
//  protocol::exceptions::GeneralError: received general error

class Header {
 protected:
  types::Version server_version_;
  types::Code code_;
  types::PayloadSize payload_size_;
  Header(const types::Code &expected_code,
         boost::asio::ip::tcp::socket &socket);
};

struct Register : public Header {
  types::ClientID client_id;
  Register(boost::asio::ip::tcp::socket &socket);
};

struct Client {
  // This structure, that is supposed to be
  // used along with the 'ClientList' class.
  types::ClientID id;
  types::Username name;
};

class ClientList : public Header {
 public:
  ClientList(boost::asio::ip::tcp::socket &&socket);

  // Reads the clients from the socket, and passes them
  // one by one to a given function.
  void ReadClients(std::function<void(Client &client)> proccess_client);

  // The amount of clients available in the socket.
  types::PayloadSize::DataType client_count() { return client_count_; }

 private:
  boost::asio::ip::tcp::socket socket_;
  types::PayloadSize::DataType client_count_;
};

struct PublicKey : public Header {
  types::ClientID target_id;
  types::PublicKey target_public_key;
  PublicKey(boost::asio::ip::tcp::socket &socket);
};

struct MessageSent : public Header {
  types::ClientID target_id;
  types::MessageID message_id;
  MessageSent(boost::asio::ip::tcp::socket &socket);
};

class Message {
 public:
  // Supposed to be used along with the 'PendingMessages' class.
  types::ClientID sender_id;
  types::MessageID id;
  types::MessageType type;

  // Throws std::runtime_error if content
  // does not exist.
  types::Content content();

  // Deletes old content if exist.
  void CreateContent(const std::string &filename);

  Message() = default;
  Message(Message &) = delete;
  Message(Message &&other);
  ~Message() { delete content_; }

 private:
  types::Content *content_ = nullptr;
};

class PendingMessages : public Header {
 public:
  PendingMessages(boost::asio::ip::tcp::socket &&socket);

  // Reads the messages from the socket, and passes them
  // one by one to a given function.
  //
  // throws ContentMismatch if the content size doesn't match
  // the payload size.
  void ReadMessages(std::function<void(Message &message)> proccess_message);

  // Checks if any messages available in the socket.
  operator bool() const { return payload_size_.value(); }

 private:
  boost::asio::ip::tcp::socket socket_;
};

}  // namespace response
}  // namespace protocol
}  // namespace messageu

#endif