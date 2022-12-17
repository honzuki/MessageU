
#include "config.hpp"

#include <fstream>
#include <regex>
#include <stdexcept>

#include "protocol/types.hpp"
#include "radix.hpp"

namespace messageu {
namespace config {

ServerInfo::ServerInfo(const std::filesystem::path &file_path) {
  // Regex pattern that matches <ip>:<port>
  const static std::regex ip_pattern(
      R"(^\s{0,}(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3}):(\d{1,4})\s{0,}$)");

  std::ifstream info_file(file_path);
  std::string content;
  if (!std::getline(info_file, content))
    throw std::invalid_argument("failed to read the server info from " +
                                file_path.string());

  // Use regex to parse the server details
  std::smatch base_match;
  if (std::regex_match(content, base_match, ip_pattern)) {
    if (base_match.size() != 3)
      throw std::invalid_argument("could not read the server info from " +
                                  file_path.string());
    ip_ = base_match[1];
    port_ = base_match[2];
  } else
    throw std::invalid_argument("could not read the server info from " +
                                file_path.string());
}

MyInfo::MyInfo(const std::string username, protocol::types::ClientID id,
               crypto::asymmetric::PrivateKey private_key)
    : username_(username),
      id_(id),
      private_key_(new crypto::asymmetric::PrivateKey(private_key)) {}

MyInfo::MyInfo(const std::filesystem::path &file_path) {
  std::ifstream info_file(file_path);
  if (!std::getline(info_file, username_))
    throw std::invalid_argument("failed to read the username from " +
                                file_path.string());
  if (username_.size() > protocol::types::kUsernameSize)
    throw std::invalid_argument("the username in the info file is too long");

  // hex converts every byte to characters
  constexpr auto stored_id_size = protocol::types::kClientIDSize * 2;
  std::string stored_id;
  if (!std::getline(info_file, stored_id) || stored_id.size() != stored_id_size)
    throw std::invalid_argument("failed to read the client_id from " +
                                file_path.string());
  auto raw_client_id = radix::hex::Decode(stored_id);
  std::copy_n(raw_client_id.begin(), protocol::types::kClientIDSize,
              id_.begin());  // copy the raw id into the id type.

  std::string stored_private_key;
  if (!std::getline(info_file, stored_private_key))
    throw std::invalid_argument("failed to read the private_key from " +
                                file_path.string());

  private_key_ = new crypto::asymmetric::PrivateKey(
      radix::base64::Decode(stored_private_key));
}

void MyInfo::Save(const std::filesystem::path &file_path) {
  std::ofstream info_file(file_path);
  info_file << username_ << "\n" << radix::hex::Encode(id_.data(), id_.size());
  if (private_key_)
    info_file << "\n" << radix::base64::Encode(private_key_->Export());
}

MyInfo::~MyInfo() { delete private_key_; }

}  // namespace config
}  // namespace messageu