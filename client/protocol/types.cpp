#include "types.hpp"

namespace messageu {
namespace protocol {
namespace types {
Content::Content(const std::string& filename)
    : reference_count_(new std::size_t),
      dump_file_(new tempfile::TempFile(filename)) {
  *reference_count_ = 1;
}

Content::Content(Content& other)
    : reference_count_(other.reference_count_), dump_file_(other.dump_file_) {
  ++(*reference_count_);
}

Content::~Content() {
  if (!(--(*reference_count_))) {
    delete reference_count_;
    delete dump_file_;
  }
}

const tempfile::TempFile& Content::operator*() const { return *dump_file_; }
const tempfile::TempFile* Content::operator->() const { return dump_file_; }

}  // namespace types
}  // namespace protocol
}  // namespace messageu