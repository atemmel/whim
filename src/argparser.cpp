#include "argparser.hpp"

#include <iostream>
#include <algorithm>

ArgParser::ArgParser(int argc, char** argv) : args(argv + 1, argv + argc) {
}

auto ArgParser::addBool(bool* var, std::string_view flag, std::string_view usage) -> void {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::Bool, usage}});
}

auto ArgParser::addString(std::string* var, std::string_view flag, std::string_view usage) -> void {
	flags.insert({flag, {static_cast<void*>(var), VarPtr::Type::String, usage}});
}

auto ArgParser::unwind() -> Result<void> {
	Result<void> result;
	for(auto it = args.begin(); it != args.end(); it++) {
		auto hashIt = flags.find(*it);

		if(hashIt == flags.end()) {
			if(*it == "-h" || *it == "--help" || *it == "help") {
				usage();
			}
			std::string err = "Unrecognized argument: ";
			err += *it;
			err += ", exiting...\n";
			return result.set(err);
		}

		auto& flag = hashIt->second;

		int availableArgs = std::distance(std::next(it), args.end() );
		if(availableArgs < 1 && flag.var.type != VarPtr::Type::Bool) {
			std::string err = "Too few arguments for argument '";
			err += hashIt->first ;
			err += "', expected 1, recieved 0";
			err += availableArgs;
			err += '\n';
			return result.set(err);
		}

		switch(flag.var.type) {
			case VarPtr::Type::Bool:
				*static_cast<bool*>(flag.var.ptr) = true;
				break;
			case VarPtr::Type::String:
				static_cast<std::string*>(flag.var.ptr)->assign(*std::next(it) );
				break;
		}

		if(flag.var.type != VarPtr::Type::Bool) std::advance(it, 1);
	}

	return result;
}

auto ArgParser::usage() const -> void {
	std::cout << "whim help:\n";
	for(const auto& it : flags) {
		std::cout << "  " << it.first << ": ";
		switch(it.second.var.type) {
			case VarPtr::Type::Bool:
				break;
			case VarPtr::Type::String:
				std::cout << "(string) ";
				break;
		}
		std::cout << it.second.usage << '\n';
	}
	std::cout << "\n  help: Display this message\n";
	std::exit(EXIT_SUCCESS);
}
