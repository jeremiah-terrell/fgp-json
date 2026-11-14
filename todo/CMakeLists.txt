add_executable(todo
  include/marshall.hpp
  include/todo.hpp
  main.cpp
  marshall.cpp
  todo.cpp
)

target_include_directories(todo PUBLIC include)

target_link_libraries(todo PRIVATE libjson)

set_property(TARGET todo PROPERTY CXX_STANDARD 20)
