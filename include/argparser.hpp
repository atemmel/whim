#pragma once

#include "result.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class ArgParser {
public:
	using Args = std::vector<std::string_view>;

	ArgParser(int argc, char** argv);

	auto addBool(bool* var, std::string_view flag) -> void;
	auto addString(std::string* var, std::string_view flag) -> void;

	auto unwind() -> Result<void>;

private:
	struct VarPtr {
		enum struct Type {
			Bool,
			String
		};
		void* ptr;
		Type type;
	};

	std::unordered_map<std::string_view, VarPtr> flags;
	Args args;
};
