// Handles tempfiles in the system
//
// All tempfiles are saved in the appropriate
// temp-file system of the OS, under a sub-folder
// creates specifically for our client.

#ifndef CLIENT_TEMPFILE_H
#define CLIENT_TEMPFILE_H

#include <filesystem>

namespace messageu {
namespace tempfile {

// The name of the sub-folder in the temp filesystem
constexpr char kTempFolderName[] = "MessageU";

// Helps avoiding collosions between 2 different clients under the same machine.
// (the size of the random part)
constexpr size_t kSystemRandomSize = 32;
constexpr size_t kFileRandomSize = 8;

class TempFile {
 public:
  // Will overwrite existing files!
  //
  // If auto_delete is off, it won't delete the file once
  // the temp file object is destroyed.
  // The file is still saved to the OS temporary filesystem,
  // so it should get deleted automatically according to the OS policy.
  TempFile(const std::string &name, bool auto_delete = true);

  // Deletes the file from the system, if auto_delete exists.
  // does nothing will happen if the file got deleted already.
  ~TempFile();

  const std::filesystem::path &path() const { return path_; }
  const std::uintmax_t size() const {
    return std::filesystem::file_size(path_);
  }

  // This can be removed if support for "copying"
  // a temp file is added; create a new file, and
  // copy the contents of the old one to the new one.
  TempFile(TempFile &) = delete;

 private:
  std::filesystem::path path_;
  bool auto_delete_;
};

}  // namespace tempfile
}  // namespace messageu

#endif