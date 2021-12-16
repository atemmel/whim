#include "markdown.hpp"

#include <iostream>

thread_local std::string_view md::view;
thread_local md::ParseState md::state; 

auto md::Node::addChild(Child& child) -> void {
	children.push_back(std::move(child));
}

auto md::Document::accept(Visitor& visitor) const -> void {
	visitor.visit(*this);
}

auto md::Paragraph::accept(Visitor& visitor) const -> void {
	visitor.visit(*this);
}

auto md::Header::accept(Visitor& visitor) const -> void {
	visitor.visit(*this);
}

auto md::Printer::visit(const Document& document) -> void {
	std::cerr << "document:\n";
	depth++;
	for(const auto& child : document.children) {
		pad();
		child->accept(*this);
	}
	depth--;
}

auto md::Printer::visit(const Paragraph& paragraph) -> void {
	std::cerr << "Paragraph: '" << paragraph.contents << "'\n";
}

auto md::Printer::visit(const Header& header) -> void {
	std::cerr << "Header: '" << header.contents 
		<< "', Level: " << header.level << '\n';
}

auto md::Printer::pad() const -> void {
	for(uint32_t i = 0; i < depth; i++) {
		std::cerr << "    ";
	}
}

auto md::parse(const std::string_view str) -> std::unique_ptr<Document> {
	state = {
		.index = 0,
		.column = 1,
		.row = 1,
	};
	view = str;
	std::unique_ptr<Document> result = std::make_unique<Document>();

	result->meta = parseMetadata();

	skipBlank();
	while(!eof()) {
		if(auto header = parseHeader(); header) {
			result->addChild(header);
		} else if(auto paragraph = parseParagraph(); paragraph) {
			result->addChild(paragraph);
		} else {
			return result;
		}
		skipBlank();
	}

	return result;
}

auto md::parseMetadata() -> std::unordered_map<std::string, std::string> {
	std::unordered_map<std::string, std::string> map;
	auto checkpoint = state;
	skipBlank();
	while(!eof() && peek() == '-') {
		state.index++;
		state.column++;
	}

	if(eof() || peek() != '\n') {
		state = checkpoint;
		return {};
	}

	state.column = 1;
	state.row++;
	state.index++;

	while(true) {

		skipBlank();

		if(eof()) {
			state = checkpoint;
			return {};
		}

		auto keyBegin = state.index;

		while(!eof() && peek() != '=') {
			next();
		}

		if(eof()) {
			state = checkpoint;
			return {};
		}
		auto keyEnd = state.index - 1;
		next();

		skipBlank();

		if(eof()) {
			state = checkpoint;
			return {};
		}

		auto valueBegin = state.index;
		while(!eof() && peek() != '\n') {
			state.index++;
			state.column++;
		}

		if(eof()) {
			state = checkpoint;
			return {};
		}

		auto valueEnd = state.index;

		while(!std::isgraph(view[keyEnd - 1])) {
			keyEnd--;
		}

		while(!std::isgraph(view[valueEnd - 1])) {
			valueEnd--;
		}

		std::string_view key(view.data() + keyBegin, keyEnd - keyBegin);
		std::string_view value(view.data() + valueBegin, valueEnd - valueBegin);
		map.emplace(key, value);

		state.column = 1;
		state.row++;
		state.index++;

		if(eof()) {
			state = checkpoint;
			return {};
		}

		if(peek() == '-') {
			break;
		}
	}

	while(!eof() && peek() == '-') {
		state.column++;
		state.index++;
	}

	next();
	
	return map;
}

auto md::parseParagraph() -> Child {
	auto checkpoint = state;
	auto paragraphBegin = state.index;

	while(!eof() && isSpecialChar(peek())) {
		next();
	}

	auto paragraphEnd = state.index;

	bool prevCharWasNewline = false;

	while(!eof() && !isSpecialChar(peek())) {
		prevCharWasNewline = peek() == '\n';
		paragraphEnd = state.index;
		next();
		if(prevCharWasNewline && peek() == '\n') {
			paragraphEnd = state.index - 1;
			next();
			break;
		}
	}

	if(paragraphBegin == paragraphEnd) {
		state = checkpoint;
		return nullptr;
	}

	auto paragraph = std::make_unique<Paragraph>();
	paragraph->contents.assign(view.begin() + paragraphBegin,
			view.begin() + paragraphEnd);
	return paragraph;
}

auto md::parseHeader() -> Child {
	auto checkpoint = state;
	if(peek() != '#') {
		return nullptr;
	}

	next();

	int level = 1;
	while(!eof() && peek() == '#') {
		state.column++;
		state.index++;
		level++;
		if(level >= 7) {
			state = checkpoint;
			return nullptr;
		}
	}

	if(peek() != ' ') {
		state = checkpoint;
		return nullptr;
	}

	while(peek() == ' ') {
		state.column++;
		state.index++;
	}

	if(peek() == '\n') {
		state = checkpoint;
		return nullptr;
	}

	auto headerBegin = state.index;
	while(peek() != '\n') {
		state.column++;
		state.index++;
	}

	auto headerEnd = state.index;

	auto header = std::make_unique<Header>();
	header->contents.assign(view.begin() + headerBegin,
			view.begin() + headerEnd);
	header->level = level;
	return header;
}

auto md::peek() -> char {
	return view[state.index];
}

auto md::next() -> void {
	state.column++;
	if(view[state.index] == '\n') {
		state.column = 1;
		state.row++;
	}
	state.index++;
}

auto md::eof() -> bool {
	return state.index >= view.size();
}

auto md::skipBlank() -> void {
	while(!eof() && !std::isgraph(peek())) {
		next();
	}
}

auto md::isSpecialChar(char prospect) -> bool {
	switch(prospect) {
		case '#':
		case '*':
			return true;
	}
	return false;
}
