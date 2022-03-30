#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

struct md {
	struct Visitor;
	struct Node;
	using Child = std::unique_ptr<Node>;

	struct Node {
		virtual ~Node() = default;
		virtual auto accept(Visitor& visitor) const -> void = 0;
		auto addChild(Child& child) -> void;
		std::vector<Child> children;
	};

	struct Document : public Node {
		auto accept(Visitor& visitor) const -> void;
		std::unordered_map<std::string, std::string> meta;
	};

	struct Paragraph : public Node {
		auto accept(Visitor& visitor) const -> void;
		std::string contents;
	};

	struct Header : public Node {
		auto accept(Visitor& visitor) const -> void;
		std::string contents;
		int level;
	};

	struct Code : public Node {
		auto accept(Visitor& visitor) const -> void;
		std::string contents;
		std::string language;
	};

	struct Visitor {
		virtual ~Visitor() = default;
		virtual auto visit(const Document& document) -> void = 0;
		virtual auto visit(const Paragraph& paragraph) -> void = 0;
		virtual auto visit(const Header& header) -> void = 0;
		virtual auto visit(const Code& code) -> void = 0;
	};

	struct Printer : Visitor {
		auto visit(const Document& document) -> void;
		auto visit(const Paragraph& paragraph) -> void;
		auto visit(const Header& header) -> void;
		auto visit(const Code& code) -> void;
	private:
		auto pad() const -> void;
		uint32_t depth = 0;
	};

	[[nodiscard]] static auto parse(std::string_view str) -> std::unique_ptr<Document>;

private:

	[[nodiscard]] static auto parseMetadata() -> std::unordered_map<std::string, std::string>;
	[[nodiscard]] static auto parseParagraph() -> Child;
	[[nodiscard]] static auto parseHeader() -> Child;
	[[nodiscard]] static auto parseCode() -> Child;

	[[nodiscard]] static auto peek() -> char;
	static auto next() -> void;
	[[nodiscard]] static auto eof() -> bool;
	static auto skipBlank() -> void;

	[[nodiscard]] static auto isSpecialChar(char prospect) -> bool;

	static thread_local std::string_view view;

	struct ParseState {
		size_t index;
		size_t column;
		size_t row;
	};

	static thread_local ParseState state;
};
