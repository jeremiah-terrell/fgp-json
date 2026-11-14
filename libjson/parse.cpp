// Parses JSON string and constructs Data object that represents contents.
// If you wrote your Data type according to specification then it should compile
// without incident.

#include "include/parse.hpp"
#include <istream>
#include <iterator>

namespace json {

// Recursive-descent parser. Each parseX() function begins at the first
// character of an X and leaves the iterator just past it. Never looks
// ahead more than one character, so any input iterator will do.
template <typename InputIt> class Parser {
      public:
	Parser(InputIt first, InputIt last) : _it{first}, _end{last} {}

	Data parseDocument() {
		Data result = parseValue();
		if (!atEnd())
			fail("Unexpected character");
		return result;
	}

      private:
	InputIt _it;
	InputIt _end;
	std::size_t _pos = 0; // for error messages

	//------------------------------------------------------
	// Helpers

	void fail(std::string const &message) const {
		throw ParseError(message, _pos);
	}

	bool atEnd() const {
		return _it == _end;
	}

	// Returns the current character, or '\0' at end of input
	char peek() const {
		return atEnd() ? '\0' : *_it;
	}

	// Returns the current character and advances past it
	char next() {
		if (atEnd())
			fail("Unexpected end of input");
		char ch = *_it;
		++_it;
		++_pos;
		return ch;
	}

	bool consume(char ch) {
		if (atEnd() || *_it != ch)
			return false;
		next();
		return true;
	}

	void expect(char ch) {
		if (!consume(ch))
			fail(std::string("Expected '") + ch + "'");
	}

	void expectWord(std::string_view word) {
		for (char ch : word) {
			if (!consume(ch))
				fail("Invalid literal");
		}
	}

	void skipWhitespace() {
		while (peek() == ' ' || peek() == '\t' || peek() == '\n' ||
		       peek() == '\r')
			next();
	}

	//------------------------------------------------------
	// Grammar

	// Skips whitespace on both sides of the value
	Data parseValue() {
		skipWhitespace();
		Data result;
		switch (peek()) {
		case '{':
			result = parseObject();
			break;
		case '[':
			result = parseArray();
			break;
		case '"':
			result = parseString();
			break;
		case 't':
			expectWord("true");
			result = true;
			break;
		case 'f':
			expectWord("false");
			result = false;
			break;
		case 'n':
			expectWord("null");
			result = null;
			break;
		default:
			if (peek() == '-' || (peek() >= '0' && peek() <= '9'))
				result = parseNumber();
			else if (atEnd())
				fail("Unexpected end of input");
			else
				fail("Unexpected character");
		}
		skipWhitespace();
		return result;
	}

	Data parseObject() {
		expect('{');
		Object result;
		skipWhitespace();
		if (consume('}'))
			return result;
		do {
			skipWhitespace();
			std::string key = parseString();
			skipWhitespace();
			expect(':');
			result.insert_or_assign(std::move(key), parseValue());
		} while (consume(','));
		expect('}');
		return result;
	}

	Data parseArray() {
		expect('[');
		Array result;
		skipWhitespace();
		if (consume(']'))
			return result;
		do {
			result.push_back(parseValue());
		} while (consume(','));
		expect(']');
		return result;
	}

	// Validates the strict JSON number grammar while copying the characters
	// into a buffer, then converts the buffer with from_chars:
	//   -? (0 | [1-9][0-9]*) (. [0-9]+)? ([eE] [+-]? [0-9]+)?
	double parseNumber() {
		std::string text;
		auto accept = [&](char ch) {
			if (!consume(ch))
				return false;
			text += ch;
			return true;
		};
		auto digits = [&] {
			std::size_t start = text.size();
			while (peek() >= '0' && peek() <= '9')
				text += next();
			return text.size() > start;
		};

		accept('-');
		if (!accept('0') && !digits())
			fail("Invalid number");
		if (accept('.') && !digits())
			fail("Invalid number");
		if (accept('e') || accept('E')) {
			accept('+') || accept('-');
			if (!digits())
				fail("Invalid number");
		}
		double value;
		auto [end, ec] = std::from_chars(
		    text.data(), text.data() + text.size(), value);
		if (ec != std::errc{})
			fail("Number out of range");
		return value;
	}

	std::string parseString() {
		expect('"');
		std::string result;
		while (!consume('"')) {
			if (atEnd())
				fail("Unterminated string");
			if (static_cast<unsigned char>(peek()) < 0x20)
				fail("Control character in string");
			if (!consume('\\')) {
				result += next();
				continue;
			}
			switch (next()) {
			case '"':
				result += '"';
				break;
			case '\\':
				result += '\\';
				break;
			case '/':
				result += '/';
				break;
			case 'b':
				result += '\b';
				break;
			case 'f':
				result += '\f';
				break;
			case 'n':
				result += '\n';
				break;
			case 'r':
				result += '\r';
				break;
			case 't':
				result += '\t';
				break;
			case 'u':
				appendUtf8(result, parseCodePoint());
				break;
			default:
				fail("Invalid escape sequence");
			}
		}
		return result;
	}

	// Called just after "\u". Characters outside the Basic Multilingual
	// Plane (e.g. emoji) are written as a UTF-16 surrogate pair:
	// \uD800-\uDBFF followed by \uDC00-\uDFFF. Combines the pair into one
	// code point.
	unsigned parseCodePoint() {
		unsigned code = parseHex4();
		if (code >= 0xDC00 && code <= 0xDFFF)
			fail("Invalid surrogate pair");
		if (code >= 0xD800 && code <= 0xDBFF) {
			if (!consume('\\') || !consume('u'))
				fail("Invalid surrogate pair");
			unsigned low = parseHex4();
			if (low < 0xDC00 || low > 0xDFFF)
				fail("Invalid surrogate pair");
			code =
			    0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
		}
		return code;
	}

	unsigned parseHex4() {
		char hex[4];
		for (char &ch : hex)
			ch = next();
		unsigned value = 0;
		auto [end, ec] = std::from_chars(hex, hex + 4, value, 16);
		if (ec != std::errc{} || end != hex + 4)
			fail("Invalid \\u escape");
		return value;
	}

	static void appendUtf8(std::string &out, unsigned code) {
		if (code < 0x80) {
			out += static_cast<char>(code);
		} else if (code < 0x800) {
			out += static_cast<char>(0xC0 | (code >> 6));
			out += static_cast<char>(0x80 | (code & 0x3F));
		} else if (code < 0x10000) {
			out += static_cast<char>(0xE0 | (code >> 12));
			out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (code & 0x3F));
		} else {
			out += static_cast<char>(0xF0 | (code >> 18));
			out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
			out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
			out += static_cast<char>(0x80 | (code & 0x3F));
		}
	}
};

// Parses a complete JSON document; throws ParseError on invalid input.
template <typename InputIt> Data parse(InputIt first, InputIt last) {
	return Parser<InputIt>{first, last}.parseDocument();
}

Data parse(std::string_view text) {
	return parse(text.begin(), text.end());
}

Data parse(std::istream &in) {
	return parse(std::istreambuf_iterator<char>{in},
		     std::istreambuf_iterator<char>{});
}

} // namespace json
