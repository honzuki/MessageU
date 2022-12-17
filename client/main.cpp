#include <iostream>

#include "client.hpp"

int main(int argc, char const* argv[]) {
  try {
    messageu::start_client();
  } catch (const std::exception& e) {
    std::cerr << "Got an unexpected error: " << e.what() << "\nexits safely..."
              << std::endl;
    return 1;
  }
  return 0;
}