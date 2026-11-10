#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <vector>

struct Task {
  int id = 0;
  std::string title;
  bool done = false;
  std::vector<std::string> tags;
  std::optional<std::string> completed;   // date completed, if done
};

std::ostream& operator<<(std::ostream&, Task const&);

struct TodoList {
  int nextId = 1;
  std::vector<Task> tasks;
 };
