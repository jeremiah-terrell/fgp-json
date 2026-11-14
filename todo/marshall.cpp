#include "include/marshall.hpp"
#include "../libjson/include/json.hpp"

#include <stdexcept>
#include <string>

using namespace std::string_literals;

namespace {

// Read a required field from a JSON object.
json::Data const &field(json::Object const &obj, std::string const &name) {
	auto it = obj.find(name);

	if (it == obj.end())
		throw std::runtime_error("Missing JSON field: " + name);

	return it->second;
}

Task unmarshallTask(json::Data const &data) {
	if (!json::is<json::Object>(data))
		throw std::runtime_error("Task must be a JSON object");

	auto const &obj = json::as<json::Object>(data);

	Task task;

	if (!json::is<json::Number>(field(obj, "id")))
		throw std::runtime_error("Task id must be a number");

	task.id = static_cast<int>(json::as<json::Number>(field(obj, "id")));

	if (!json::is<json::String>(field(obj, "title")))
		throw std::runtime_error("Task title must be a string");

	task.title = json::as<json::String>(field(obj, "title"));

	if (!json::is<json::Boolean>(field(obj, "done")))
		throw std::runtime_error("Task done must be a boolean");

	task.done = json::as<json::Boolean>(field(obj, "done"));

	auto const &tags = field(obj, "tags");

	if (!json::is<json::Array>(tags))
		throw std::runtime_error("Task tags must be an array");

	for (auto const &tag : json::as<json::Array>(tags)) {
		if (!json::is<json::String>(tag))
			throw std::runtime_error("Task tag must be a string");

		task.tags.push_back(json::as<json::String>(tag));
	}

	auto const &completed = field(obj, "completed");

	if (json::is<json::Null>(completed)) {
		task.completed = std::nullopt;
	} else if (json::is<json::String>(completed)) {
		task.completed = json::as<json::String>(completed);
	} else {
		throw std::runtime_error(
		    "Task completed must be a string or null");
	}

	return task;
}

} // namespace

json::Data marshalll(TodoList const &list) {
	json::Array tasks;

	for (auto const &task : list.tasks) {
		json::Object obj{{"id", task.id},
				 {"title", task.title},
				 {"done", task.done},
				 {"tags", json::Array{}},
				 {"completed", json::null}};

		json::Array tags;

		for (auto const &tag : task.tags)
			tags.emplace_back(tag);

		obj["tags"] = std::move(tags);

		if (task.completed)
			obj["completed"] = *task.completed;

		tasks.emplace_back(std::move(obj));
	}

	json::Object result{{"nextId", list.nextId},
			    {"tasks", std::move(tasks)}};

	return result;
}

TodoList unmarshalll(json::Data const &data) {
	if (!json::is<json::Object>(data))
		throw std::runtime_error("TodoList must be a JSON object");

	auto const &obj = json::as<json::Object>(data);

	TodoList list;

	auto const &nextId = field(obj, "nextId");

	if (!json::is<json::Number>(nextId))
		throw std::runtime_error("nextId must be a number");

	list.nextId = static_cast<int>(json::as<json::Number>(nextId));

	auto const &tasks = field(obj, "tasks");

	if (!json::is<json::Array>(tasks))
		throw std::runtime_error("tasks must be an array");

	for (auto const &task : json::as<json::Array>(tasks))
		list.tasks.push_back(unmarshallTask(task));

	return list;
}
