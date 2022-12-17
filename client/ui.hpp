// A simple console ui.

#ifndef CLIENT_UI_H
#define CLIENT_UI_H

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <map>
#include <string>

namespace messageu {
namespace ui {

struct Command {
  std::string title;
  std::function<void(std::ostream& output)> callback;
};

class UI {
 public:
  UI(const std::string& title) : title_(title) {}

  // Registers a new cmd to the UI
  //
  // Args:
  //  trigger:
  //    an integral key that triggers this cmd.
  //    the trigger key is unique, and it'll overwrite
  //    an old one, if exists.
  //  title:
  //    a simple textual explanation of the cmd
  //  callback:
  //    the UI will call the callback function every time the user
  //    triggers this cmd. the function gets an output stream to
  //    which it can display content to the user.
  void RegisterCmd(std::size_t trigger, const std::string& title,
                   std::function<void(std::ostream& output)> callback);

  // Reads a line from the user
  std::string ReadLine(const std::string& prompt);

  // Runs the UI in a loop.
  //
  // Args:
  //  exit_trigger:
  //    a trigger to the cmd that will exit the UI.
  //    be aware that it'll overwrite existing key.
  void Run(std::size_t exit_trigger);

 private:
  std::string title_;
  std::map<std::size_t, Command> triggers_;
};

}  // namespace ui
}  // namespace messageu

#endif