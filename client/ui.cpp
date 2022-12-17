#include "ui.hpp"

#include <limits>

namespace messageu {
namespace ui {

void UI::RegisterCmd(std::size_t trigger, const std::string& title,
                     std::function<void(std::ostream& output)> callback) {
  triggers_[trigger] = Command{.title = title, .callback = callback};
}

std::string UI::ReadLine(const std::string& prompt) {
  std::string line;
  std::cout << prompt;
  // Read until you receieve a non-empty line
  while (!std::cin.eof() && !std::cin.bad() && !line.size())
    std::getline(std::cin, line);
  return line;
}

void UI::Run(std::size_t exit_trigger) {
  // Make sure no trigger registered to the exit trigger
  try {
    triggers_.erase(exit_trigger);
  } catch (const std::out_of_range&) {
    // do nothing
  }

  while (true) {
    // Display the menu
    std::cout << "\n\n" << title_ << "\n\n";
    for (const auto& [trigger, command] : triggers_)
      std::cout << trigger << ") " << command.title << "\n";
    std::cout << exit_trigger << ") Exit client\nYour choice: ";

    // Parse request
    std::size_t trigger;
    std::cin >> trigger;
    if (std::cin.bad() || std::cin.eof()) {
      return;
    } else if (std::cin.fail()) {
      std::cout << "Unknown cmd!\n" << std::endl;
      // Ignore the whole line
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (trigger == exit_trigger) {
      return;
    } else {
      try {
        auto cmd = triggers_.at(trigger);
        cmd.callback(std::cout);
      } catch (const std::out_of_range&) {
        std::cout << "Unknown cmd!" << std::endl;
      }
    }
  }
}

}  // namespace ui
}  // namespace messageu