// Handles the config files, and their loading proccess

#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <filesystem>
#include <string>

#include "crypto/asymmetric.hpp"
#include "crypto/symmetric.hpp"
#include "protocol/types.hpp"

namespace messageu {
namespace config {

class ServerInfo {
 public:
  // Loads the info from a file
  // Throws:
  // std::invalid_argument if it can not open the file,
  //    or fails to read the info from the file.
  ServerInfo(const std::filesystem::path &file_path);

  const std::string &ip() { return ip_; };
  const std::string &port() { return port_; };

 private:
  std::string ip_;
  std::string port_;
};

class MyInfo {
 public:
  MyInfo(const std::string username, protocol::types::ClientID id,
         crypto::asymmetric::PrivateKey private_key);

  // Loads the info from a file
  //
  // Throws:
  // std::invalid_argument if it can not open the file,
  //    or fails to read the info from the file.
  MyInfo(const std::filesystem::path &file_path);
  ~MyInfo();

  // Saves the info file to the given path
  //
  // Throws:
  // std::invalid_argument if it can not write to the file
  void Save(const std::filesystem::path &file_path);

  const std::string &username() { return username_; }
  const protocol::types::ClientID &client_id() { return id_; }
  const crypto::asymmetric::PrivateKey &private_key() {
    // We're guranteed that the pointer will be valid
    return *private_key_;
  }

  // private key isn't shareable
  MyInfo(MyInfo &) = delete;

 private:
  std::string username_;
  protocol::types::ClientID id_;
  crypto::asymmetric::PrivateKey *private_key_ = nullptr;
};

}  // namespace config
}  // namespace messageu

#endif