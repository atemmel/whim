#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

struct md {
	struct Node;
	using Child = std::unique_ptr<Node>;

	struct Node {
		virtual ~Node() = default;
		std::vector<Child> children;
	};

	struct Document : public Node {
		std::unordered_map<std::string, std::string> meta;
	};

	static auto parse(std::string_view str) -> std::unique_ptr<Document>;

private:

	static auto parseMetadata() -> std::unordered_map<std::string, std::string>;

	static auto peek() -> char;
	static auto next() -> void;
	static auto eof() -> bool;
	static auto skipBlank() -> void;

	static thread_local std::string_view view;

	struct ParseState {
		size_t index;
		size_t column;
		size_t row;
	};

	static thread_local ParseState state;
};
