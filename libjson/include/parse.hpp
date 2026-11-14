#pragma once

// Parses JSON string and constructs Data object that represents contents.
// If you wrote your Data type according to specification then it should compile
// without incident.

#include "json.hpp"
#include <charconv>
#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <string_view>

namespace json {

// Error object thrown on parse error
class ParseError : public std::runtime_error {
      public:
	ParseError(std::string const &message, std::size_t position)
	    : runtime_error(message + " at position " +
			    std::to_string(position)),
	      position{position} {}

	std::size_t position;
};

Data parse(std::string_view text);

Data parse(std::istream &in);

} // namespace json
