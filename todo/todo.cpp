#include "include/todo.hpp"
#include <format>
#include <iomanip>

std::ostream &operator<<(std::ostream &out, Task const &task) {
	out << (task.done ? "[x] " : "[ ] ") << std::format("{:>3}  ", task.id)
	    << std::quoted(task.title);
	for (auto &tag : task.tags) {
		out << "  #" << tag;
	}
	if (task.completed) {
		out << "  (done " << *task.completed << ')';
	}
	return out;
}
