add_executable(todo
  include/marshal.hpp
  include/todo.hpp
  main.cpp
  marshal.cpp
  todo.cpp
)

target_include_directories(todo PUBLIC include)

target_link_libraries(todo PRIVATE libjson)

set_property(TARGET todo PROPERTY CXX_STANDARD 20)
