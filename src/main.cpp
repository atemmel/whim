#include "argparser.hpp"
#include "build.hpp"
#include "live_reload.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int {

	std::string liveReloadProjectPath;
	std::string buildProjectPath;
	std::string outputPath = "./";
	bool verboose = false;

	ArgParser argParser(argc, argv);
	argParser.addString(&liveReloadProjectPath, "serve", "Serve a file for real-time editing");
	argParser.addString(&buildProjectPath, "build", "Build a project");
	argParser.addString(&outputPath, "--output", "Set output path for build");
	argParser.addBool(&verboose, "--verboose", "Print additional information about what is going on");
	auto unwind = argParser.unwind();
	if(unwind.fail()) {
		std::cerr << unwind.reason();
		return EXIT_FAILURE;
	}

	if(liveReloadProjectPath.empty() && buildProjectPath.empty()) {
		argParser.usage();
	}

	if(!buildProjectPath.empty()) {
		return doBuildProject(buildProjectPath, outputPath, verboose);
	}
	
	return doLiveReload(liveReloadProjectPath);
}
