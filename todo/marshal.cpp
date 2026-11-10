#include "marshal.hpp"
#include "json.hpp"


//----------------------------------------------------------
// These are the only two functions exported by this module:

// Constructs JSON Data object corresponding to TodoList.
json::Data marshal(TodoList const& list) {
  return { };
}


// Constructs TodoList corresponding to JSON Data.  Throws std::runtime_error if the JSON Data does
// not correctly describe a to-do list.
TodoList unmarshal(json::Data const& data) {
  return { };
}


// You are encouraged to write additional functions to use
// as you are implementing the two above.
//----------------------------------------------------------
