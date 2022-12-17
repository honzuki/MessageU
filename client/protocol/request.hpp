#ifndef CLIENT_PROTOCOL_REQUEST_H
#define CLIENT_PROTOCOL_REQUEST_H

#include <boost/asio.hpp>

#include "types.hpp"

namespace messageu {
namespace protocol {
namespace request {

constexpr types::Code::DataType kRegisterCode = 1100, kClientListCode = 1101,
                                kPublicKeyCode = 1102, kSendMessagesCode = 1103,
                                kRetrievePendingMessageCode = 1104;

const types::Version kClientVersion = 2;

class Header {
 public:
  // Serializes the request into an open socket.
  virtual void send(boost::asio::ip::tcp::socket &socket) const;

 protected:
  Header(const types::ClientID &sender_id, const types::Code &code,
         const types::PayloadSize &payload_size);
  virtual ~Header() = default;

 private:
  types::ClientID sender_id_;
  types::Code code_;
  types::PayloadSize payload_size_;
};

class Register : public Header {
 public:
  Register(const types::Username &username, const types::PublicKey &public_key);
  void send(boost::asio::ip::tcp::socket &socket) const override;

 private:
  static types::ClientID dump_id_;
  types::Username username_;
  types::PublicKey public_key_;
};

class ClientList : public Header {
 public:
  ClientList(const types::ClientID &sender_id);
  void send(boost::asio::ip::tcp::socket &socket) const override;
};

class GetPublicKey : public Header {
 public:
  GetPublicKey(const types::ClientID &sender_id,
               const types::ClientID &target_id);
  void send(boost::asio::ip::tcp::socket &socket) const override;

 private:
  types::ClientID target_id_;
};

class SendMessage : public Header {
 public:
  SendMessage(const types::ClientID &sender_id,
              const types::ClientID &target_id, const types::MessageType &type,
              types::Content content);
  void send(boost::asio::ip::tcp::socket &socket) const override;

 private:
  types::ClientID target_id_;
  types::MessageType type_;
  types::Content content_;
};

class RetrievePendingMessages : public Header {
 public:
  RetrievePendingMessages(const types::ClientID &sender_id);
  void send(boost::asio::ip::tcp::socket &socket) const override;
};

}  // namespace request
}  // namespace protocol
}  // namespace messageu

#endif