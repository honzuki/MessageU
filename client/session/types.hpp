#ifndef CLIENT_SESSION_TYPES_H
#define CLIENT_SESSION_TYPES_H

#include <filesystem>
#include <ostream>

#include "../crypto/asymmetric.hpp"
#include "../crypto/symmetric.hpp"
#include "../tempfile.hpp"

namespace messageu {
namespace session {

class Session;

namespace types {

// As a user of the message-classes,
// all you need to know is that a message
// object can be piped into an output stream.
class Message {
  friend Session;
  friend std::ostream &operator<<(std::ostream &, const Message &);

 public:
  virtual ~Message() {}

 protected:
  Message(const std::string &sender_name) : sender_name_(sender_name) {}

  // Outputs the message to an open ostream.
  virtual void Display(std::ostream &) const = 0;

  std::string sender_name_;
};

class SymmetricKeyRequestMessage : public Message {
  friend Session;

 protected:
  void Display(std::ostream &ostream) const;

 private:
  SymmetricKeyRequestMessage(const std::string &sender_name)
      : Message(sender_name) {}
};

class ReceivedSymmetricKeyMessage : public Message {
  friend Session;

 protected:
  void Display(std::ostream &ostream) const;

 private:
  ReceivedSymmetricKeyMessage(const std::string &sender_name)
      : Message(sender_name) {}
};

class FileMessage : public Message {
  friend Session;

 public:
  FileMessage(FileMessage &other);
  ~FileMessage() { delete dump_file_; }

 protected:
  void Display(std::ostream &ostream) const;

  // The class takes care of releasing the pointer
  // (and essentially deleting the file)
  FileMessage(const std::string &sender_name, tempfile::TempFile *dump_file_)
      : Message(sender_name), dump_file_(dump_file_) {}

  tempfile::TempFile *dump_file_;
};

class TextMessage : public FileMessage {
  friend Session;

 protected:
  // Check FileMessage constructor for more info
  TextMessage(const std::string &sender_name, tempfile::TempFile *dump_file_)
      : FileMessage(sender_name, dump_file_) {}

  void Display(std::ostream &ostream) const;
};

// When the session can't decrypt the message.
class EncryptedMessage : public Message {
  friend Session;

 protected:
  void Display(std::ostream &ostream) const;

 private:
  EncryptedMessage(const std::string &sender_name) : Message(sender_name) {}
};

class ErrorMessage : public Message {
  friend Session;

 protected:
  void Display(std::ostream &ostream) const;

 private:
  ErrorMessage(const std::string &sender_name, const std::string &reason)
      : Message(sender_name), reason_(reason) {}
  std::string reason_;
};

// Can't use a struct because the keys are pointers
// and we aren't allowed to use smart-pointers.
class Client {
 public:
  Client(const protocol::types::ClientID &id, const std::string &username)
      : id_(id), username_(username) {}

  const protocol::types::ClientID &id() const { return id_; };
  const std::string &username() const { return username_; };

  // Throws session::exceptions::MissingKey if there is no symmetric_key
  const crypto::symmetric::Key &symmetric_key() const;

  // Overwrites the old one if exists
  void set_symmetric_key(const crypto::symmetric::Key &key);

  // Throws session::exceptions::MissingKey if there is no public_key
  const crypto::asymmetric::PublicKey &public_key() const;

  // Overwrites the old one if exists
  void set_public_key(const crypto::asymmetric::PublicKey &key);

  ~Client();

  // symmetric_key is unshareble
  Client(Client &) = delete;

 private:
  protocol::types::ClientID id_;
  std::string username_;
  crypto::symmetric::Key *symmetric_key_ = nullptr;
  crypto::asymmetric::PublicKey *public_key_ = nullptr;
};

}  // namespace types
}  // namespace session
}  // namespace messageu

#endif