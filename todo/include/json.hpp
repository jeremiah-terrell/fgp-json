#pragma once

#include <compare>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace json {

class Data;

using Null = std::monostate;
using Boolean = bool;
using Number = double;
using String = std::string;
using Array = std::vector<Data>;
using Object = std::map<std::string, Data>;

inline constexpr Null null{};

class Data {
      public:
	using Variant =
	    std::variant<Null, Boolean, Number, String, Array, Object>;

      private:
	Variant value;

      public:
	Data() : value(null) {}

	Data(Null v) : value(v) {}

	template <std::same_as<bool> B> Data(B v) : value(v) {}

	Data(Number v) : value(v) {}

	Data(String const &v) : value(v) {}

	Data(String &&v) : value(std::move(v)) {}

	Data(char const *v) : value(String(v)) {}

	Data(Array const &v) : value(v) {}

	Data(Array &&v) : value(std::move(v)) {}

	Data(Object const &v) : value(v) {}

	Data(Object &&v) : value(std::move(v)) {}

	Data(Data const &) = default;
	Data(Data &&) = default;

	Data &operator=(Data const &) = default;
	Data &operator=(Data &&) = default;

	Data &operator=(Null v) {
		value = v;
		return *this;
	}

	template <std::same_as<bool> B> Data &operator=(B v) {
		value = v;
		return *this;
	}

	Data &operator=(Number v) {
		value = v;
		return *this;
	}

	Data &operator=(String const &v) {
		value = v;
		return *this;
	}

	Data &operator=(String &&v) {
		value = std::move(v);
		return *this;
	}

	Data &operator=(char const *v) {
		value = String(v);
		return *this;
	}

	Data &operator=(Array const &v) {
		value = v;
		return *this;
	}

	Data &operator=(Array &&v) {
		value = std::move(v);
		return *this;
	}

	Data &operator=(Object const &v) {
		value = v;
		return *this;
	}

	Data &operator=(Object &&v) {
		value = std::move(v);
		return *this;
	}

	Variant &variant() & {
		return value;
	}

	Variant const &variant() const & {
		return value;
	}

	Variant &&variant() && {
		return std::move(value);
	}

	bool operator==(Data const &) const = default;

	std::partial_ordering operator<=>(Data const &other) const {
		return value <=> other.value;
	}
};

template <typename T> bool is(Data const &d) {
	return std::holds_alternative<T>(d.variant());
}

template <typename T> T &as(Data &d) {
	return std::get<T>(d.variant());
}

template <typename T> T const &as(Data const &d) {
	return std::get<T>(d.variant());
}

template <typename T> T &&as(Data &&d) {
	return std::get<T>(std::move(d).variant());
}

struct Formatter {
	Data const &data;
	int indent;
};

inline Formatter format(Data const &data, int indent = 0) {
	return Formatter{data, indent};
}

class Printer {
      private:
	std::ostream &out;
	int indent;
	int level;

	void newline() {
		if (indent > 0) {
			out << '\n';
			out << std::string(level * indent, ' ');
		}
	}

      public:
	Printer(std::ostream &stream, int spaces)
	    : out(stream), indent(spaces), level(0) {}

	void print(Null) {
		out << "null";
	}

	void print(Boolean b) {
		out << (b ? "true" : "false");
	}

	void print(Number n) {
		out << n;
	}

	void print(String const &s) {
		out << std::quoted(s);
	}

	void print(Array const &arr) {
		out << '[';

		if (!arr.empty()) {
			++level;

			for (std::size_t i = 0; i < arr.size(); ++i) {
				newline();

				if (i > 0) {
				}

				if (i > 0) {
				}

				printData(arr[i]);

				if (i + 1 < arr.size()) {
					out << ',';
				}
			}

			--level;
			newline();
		}

		out << ']';
	}

	void print(Object const &obj) {
		out << '{';

		if (!obj.empty()) {
			++level;

			std::size_t i = 0;

			for (auto const &[key, val] : obj) {
				newline();

				out << std::quoted(key) << ':';

				if (indent > 0) {
					out << ' ';
				}

				printData(val);

				if (++i < obj.size()) {
					out << ',';
				}
			}

			--level;
			newline();
		}

		out << '}';
	}

	void printData(Data const &d) {
		std::visit([this](auto const &v) { print(v); }, d.variant());
	}
};

inline std::ostream &operator<<(std::ostream &out, Data const &data) {
	Printer printer(out, 0);
	printer.printData(data);
	return out;
}

inline std::ostream &operator<<(std::ostream &out, Formatter const &f) {
	Printer printer(out, f.indent);
	printer.printData(f.data);
	return out;
}

} // namespace json

#include "parse.hpp"
