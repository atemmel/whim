#include "argparser.hpp"
#include "live_reload.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int {

	std::string liveReloadProjectPath;

	ArgParser argParser(argc, argv);
	argParser.addString(&liveReloadProjectPath, "serve", "Serve a file for real-time editing");
	auto parseResult = argParser.unwind();
	if(parseResult.fail()) {
		std::cerr << parseResult.reason();
		return EXIT_FAILURE;
	}

	if(liveReloadProjectPath.empty()) {
		argParser.usage();
	}
	
	return doLiveReload(liveReloadProjectPath);
}
