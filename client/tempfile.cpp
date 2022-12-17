
#include "tempfile.hpp"

#include <time.h>

#include <cstddef>
#include <fstream>
#include <iosfwd>
#include <random>

namespace messageu {

namespace {

std::string random_name(std::size_t size) {
  static std::string chars(
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "1234567890");
  static std::uniform_int_distribution<unsigned> index_dist(0,
                                                            chars.size() - 1);
  static std::random_device rd;
  static std::default_random_engine rng_engine(rd());

  std::string filename;
  for (size_t i = 0; i < size; ++i)
    filename.push_back(chars[index_dist(rng_engine)]);
  return filename;
}

}  // namespace

namespace tempfile {
TempFile::TempFile(const std::string &name, bool auto_delete)
    : auto_delete_(auto_delete) {
  static auto temp_subsystem = std::filesystem::temp_directory_path() /
                               kTempFolderName / random_name(kSystemRandomSize);
  std::filesystem::create_directories(temp_subsystem);
  path_ = temp_subsystem / (random_name(kSystemRandomSize) + "_" + name);

  // Generate the file in the system
  std::ofstream{path_.string()};
}

TempFile::~TempFile() {
  try {
    if (!size() || auto_delete_) std::remove(path_.string().c_str());
  } catch (const std::exception &) {
    // it doesn't matter if we can't delete the file
  }
}

}  // namespace tempfile
}  // namespace messageu