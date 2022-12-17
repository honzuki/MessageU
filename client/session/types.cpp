#include "types.hpp"

#include <fstream>

#include "../protocol/types.hpp"
#include "exceptions.hpp"

namespace messageu {
namespace session {
namespace types {

std::ostream &operator<<(std::ostream &ostream, const Message &message) {
  ostream << "\nFrom: " << message.sender_name_ << "\nContent:\n";
  message.Display(ostream);
  ostream << "\n====<EOM>====" << std::endl;

  return ostream;
}

void SymmetricKeyRequestMessage::Display(std::ostream &ostream) const {
  ostream << "Request for symmetric key";
}

void ReceivedSymmetricKeyMessage::Display(std::ostream &ostream) const {
  ostream << "Received symmetric key";
}

FileMessage::FileMessage(FileMessage &other)
    : FileMessage(other.sender_name_, other.dump_file_) {
  other.dump_file_ = nullptr;
}

void TextMessage::Display(std::ostream &ostream) const {
  std::ifstream content_file(dump_file_->path());

  while (content_file) {
    char data[protocol::types::kBlockSize]{0};
    content_file.read(data, protocol::types::kBlockSize);
    if (content_file.gcount())  // write as much as you actually read
      ostream.write(data, content_file.gcount());
  }
}

void FileMessage::Display(std::ostream &ostream) const {
  ostream << "[File] " << dump_file_->path().string();
}

void EncryptedMessage::Display(std::ostream &ostream) const {
  ostream << "Can not decrypt this message";
}

void ErrorMessage::Display(std::ostream &ostream) const {
  ostream << "[ERROR] " << reason_;
}

const crypto::symmetric::Key &Client::symmetric_key() const {
  if (!symmetric_key_) throw exceptions::MissingKey("symmetric");
  return *symmetric_key_;
}

const crypto::asymmetric::PublicKey &Client::public_key() const {
  if (!public_key_) throw exceptions::MissingKey("public");
  return *public_key_;
}

void Client::set_symmetric_key(const crypto::symmetric::Key &key) {
  if (symmetric_key_) delete symmetric_key_;
  symmetric_key_ = new crypto::symmetric::Key(key);
}

void Client::set_public_key(const crypto::asymmetric::PublicKey &key) {
  if (public_key_) delete public_key_;
  public_key_ = new crypto::asymmetric::PublicKey(key);
}

Client::~Client() {
  delete symmetric_key_;
  delete public_key_;
}

}  // namespace types
}  // namespace session
}  // namespace messageu