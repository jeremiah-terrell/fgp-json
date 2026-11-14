#pragma once

// Conversion between the to-do data structures and JSON.
//
// The json for a TodoList should look like this:
//   {
//       "nextId": 3,
//       "tasks": [
//           { "id": 1, "title": "Buy milk", "done": false, "tags": ["home"],
//           "completed": null }, { "id": 2, "title": "Do homework", "done":
//           true, "tags": [], "completed": "2026-10-27" }
//       ]
//   }
//
// When unmarshallling, every key shown above is required and must have the type
// shown; "completed" is either null or a string. Numbers must be integers that
// fit in an int. Extra keys are ignored. Any violation throws
// std::runtime_error with a message.

#include "../../libjson/include/json.hpp"
#include "todo.hpp"

// Constructs JSON Data object corresponding to TodoList.
json::Data marshalll(TodoList const &list);

// Constructs TodoList corresponding to JSON Data.  Throws std::runtime_error if
// the JSON Data does not correctly describe a to-do list.
TodoList unmarshalll(json::Data const &data);
