#include "markdown.hpp"

thread_local std::string_view md::view;
thread_local md::ParseState md::state; 

auto md::parse(const std::string_view str) -> std::unique_ptr<Document> {
	state = {
		.index = 0,
		.column = 1,
		.row = 1,
	};
	view = str;
	std::unique_ptr<Document> result = std::make_unique<Document>();

	result->meta = parseMetadata();

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
