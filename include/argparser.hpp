#pragma once

#include "result.hpp"

#include <string>
#include <string_view>
#include <map>
#include <vector>

class ArgParser {
public:
	using Args = std::vector<std::string_view>;

	ArgParser(int argc, char** argv);

	auto addBool(bool* var, std::string_view flag, std::string_view usage) -> void;
	auto addString(std::string* var, std::string_view flag, std::string_view usage) -> void;

	auto unwind() -> Result<void>;
	auto usage() const -> void;

private:
	struct VarPtr {
		enum struct Type {
			Bool,
			String
		};
		void* ptr;
		Type type;
	};

	struct Flag {
		VarPtr var;
		std::string_view usage;
	};

	std::map<std::string_view, Flag> flags;
	Args args;
};
