
#---------------------------------------
# Version with source files
add_library(libjson STATIC
  include/json.hpp
  include/parse.hpp
  parse.cpp
)

target_include_directories(libjson PUBLIC "include")

target_compile_features(libjson PUBLIC cxx_std_20)
