// A small command-line to-do list that stores its tasks in a JSON file.
//
// Usage: todo [--file <path>] <command> [args...]
//   add <title> [tag...]     Add a new task
//   list [tag]               List tasks, optionally only those with a tag
//   done <id>                Mark a task complete
//   undo <id>                Mark a task incomplete
//   rename <id> <title>      Change a task's title
//   remove <id>              Delete a task

#include "include/todo.hpp"
#include "include/marshal.hpp"
#include "../libjson/include/json.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

//----------------------------------------------------------
// Helpers

// Load TodoList from data file
TodoList loadData(std::string const &path) {
	std::ifstream in(path);
	if (!in)
		return {}; // first run: no file yet
	return unmarshall(json::parse(in));
}

// Save TodoList to data file
void saveData(std::string const &path, TodoList const &list) {
	std::ofstream out(path);
	if (!out)
		throw std::runtime_error("Cannot write " + path);
	out << json::format(marshall(list), 4) << '\n';
}

// Test whether range contains specified value
template <std::ranges::input_range R, typename V>
bool contains(R const &range, V const &value) {
	return std::ranges::find(range, value) != end(range);
}

// Parse an integer id from the command line
int parseId(std::string const &text) {
	std::size_t used = 0;
	int id = std::stoi(text, &used);
	if (used != text.size())
		throw std::invalid_argument("Invalid id: " + text);
	return id;
}

// Find specified task in task list
std::vector<Task>::iterator findTask(TodoList &list, int id) {
	auto it = std::ranges::find(list.tasks, id, &Task::id);
	if (it == list.tasks.end())
		throw std::runtime_error(std::format("No task with id {}", id));
	return it;
}

// Create current timestamp
std::string today() {
	auto now = std::chrono::floor<std::chrono::days>(
	    std::chrono::system_clock::now());
	std::chrono::year_month_day date{now};
	return std::format("{:04}-{:02}-{:02}", static_cast<int>(date.year()),
			   static_cast<unsigned>(date.month()),
			   static_cast<unsigned>(date.day()));
}

//----------------------------------------------------------
// Commands

// Add new item to list
void add(TodoList &list, std::string title, std::vector<std::string> tags) {
	std::ranges::sort(tags);
	auto duplicates = std::ranges::unique(tags);
	tags.erase(duplicates.begin(), duplicates.end());

	int id = list.nextId++;
	list.tasks.push_back({.id = id,
			      .title = std::move(title),
			      .tags = std::move(tags),
			      .completed = std::nullopt});
	std::cout << "Added task " << id << "\n";
}

// List all items with a certain tag
void list(TodoList const &list, std::string const &tagFilter) {
	int shown = 0;

	for (auto &task : list.tasks) {
		if (tagFilter.empty() || contains(task.tags, tagFilter)) {

			std::cout << task << '\n';
			++shown;
		}
	}

	if (shown == 0)
		std::cout << "No tasks.\n";
}

// Mark a task as completed or not
void setDone(TodoList &list, int id, bool done) {
	Task &task = *findTask(list, id);
	std::cout << std::format("Task {} marked {}\n", id,
				 done ? "done" : "not done");
	task.done = done;
	if (done)
		task.completed = today();
	else
		task.completed.reset();
}

// Rename existing task
void rename(TodoList &list, int id, std::string title) {
	Task &task = *findTask(list, id);
	std::cout << std::format("Renamed task {}: \"{}\" -> \"{}\"\n", id,
				 task.title, title);
	task.title = std::move(title);
}

// Remove existing task from list
void remove(TodoList &list, int id) {
	auto it = findTask(list, id);
	std::cout << std::format("Removed task {}: \"{}\"\n", id, it->title);
	list.tasks.erase(it);
}

// Print usage message
int usage() {
	std::cerr << "Usage: todo [--file <path>] <command> [args...]\n"
		     "  add <title> [tag...]     Add a new task\n"
		     "  list [tag]               List tasks, optionally only "
		     "those with a tag\n"
		     "  done <id>                Mark a task complete\n"
		     "  undo <id>                Mark a task incomplete\n"
		     "  rename <id> <title>      Change a task's title\n"
		     "  remove <id>              Delete a task\n";
	return 2;
}

//----------------------------------------------------------
// Main

int main(int argc, char *argv[]) {
	std::vector<std::string> args(argv + 1, argv + argc);
	std::string path = "todo.json";

	if (args.size() >= 2 && args[0] == "--file") {
		path = args[1];
		args.erase(args.begin(), args.begin() + 2);
	}
	if (args.empty())
		return usage();

	std::string const &command = args[0];
	std::size_t count = args.size() - 1;

	TodoList todo;
	try {
		todo = loadData(path);
	} catch (std::exception const &e) {
		std::cerr << "Cannot read " << path << ": " << e.what() << '\n';
		return 1;
	}

	try {
		bool modified = true;

		if (command == "add" && count >= 1) {
			add(todo, args[1], {args.begin() + 2, args.end()});
		} else if (command == "list" && count <= 1) {
			list(todo, count == 1 ? args[1] : "");
			modified = false;
		} else if (command == "done" && count == 1) {
			setDone(todo, parseId(args[1]), true);
		} else if (command == "undo" && count == 1) {
			setDone(todo, parseId(args[1]), false);
		} else if (command == "rename" && count == 2) {
			rename(todo, parseId(args[1]), args[2]);
		} else if (command == "remove" && count == 1) {
			remove(todo, parseId(args[1]));
		} else {
			return usage();
		}

		if (modified)
			saveData(path, todo);
	} catch (std::exception const &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
