#include "argparser.hpp"

#include <iostream>
#include <algorithm>

ArgParser::ArgParser(int argc, char** argv) : args(argv + 1, argv + argc) {
}

auto ArgParser::addBool(bool* var, std::string_view flag) -> void {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::Bool} });
}

auto ArgParser::addString(std::string* var, std::string_view flag) -> void {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::String} });
}

auto ArgParser::unwind() -> Result<void> {
	Result<void> result;
	for(auto it = args.begin(); it != args.end(); it++) {
		auto hashIt = flags.find(*it);

		if(hashIt == flags.end() ) {
			std::string err = "Unrecognized argument: ";
			err += *it;
			err += ", exiting...\n";
			return result.set(err);
		}

		auto& var = hashIt->second;

		int availableArgs = std::distance(std::next(it), args.end() );
		if(availableArgs < 1 && var.type != VarPtr::Type::Bool) {
			std::string err = "Too few arguments for argument ";
			err += hashIt->first ;
			err += ", expected 1, recieved ";
			err += availableArgs;
			err += '\n';
			return result.set(err);
		}

		switch(var.type) {
			case VarPtr::Type::Bool:
				*static_cast<bool*>(var.ptr) = true;
				break;
			case VarPtr::Type::String:
				static_cast<std::string*>(var.ptr)->assign(*std::next(it) );
				break;
		}

		if(var.type != VarPtr::Type::Bool) std::advance(it, 1);
	}

	return result;
}
