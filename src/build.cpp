#include "build.hpp"

#include "markdown.hpp"
#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

auto doBuildProject(std::string_view buildPath, std::string_view outputPath, bool verboose) -> int {
	BuildProjectState state;
	state.buildPath = buildPath;
	state.outputPath = outputPath;

	for(const auto& it : fs::recursive_directory_iterator(buildPath)) {
		if(!it.is_regular_file()) {
			continue;
		}

		if(it.path().extension() == ".md" || it.path().extension() == ".MD") {
			state.targets.push_back(it.path());
		}
	}

	for(const auto& target : state.targets) {
		buildFile(state, target, verboose);
	}

	return EXIT_SUCCESS;
}

auto buildFile(BuildProjectState& state, std::string_view file, bool verboose) -> void {
	auto mdSrc = consumeFile(file.data());
	auto mdDoc = md::parse(mdSrc);
	if(!mdDoc) {
		std::cerr << "Unable to build " << file << '\n';
		return;
	}

	auto templ = mdDoc->meta.find("template");
	if(templ == mdDoc->meta.end()) {
		std::cerr << file << " did not specify a template in header\n";
		return;
	}

	auto afterFileStem = file.find_last_of(".");
	auto afterLastSep = file.find_last_of(fs::path::preferred_separator) + 1;
	//auto afterFirstSep = file.find_first_of(fs::path::preferred_separator) + 1;
	auto name = std::string_view(file.data() + afterLastSep,
			afterFileStem - afterLastSep);

	auto delta = afterLastSep - state.buildPath.size();
	std::string fullOutputPath;
	fullOutputPath += state.outputPath;
	if(delta > 1) {
		fullOutputPath += std::string_view(file.data() + state.buildPath.size() + 1, delta - 1);
		std::cerr << "Checking if " << fullOutputPath << " exists\n";
		if(!fs::exists(fullOutputPath)) {
			std::cerr << fullOutputPath << " did not exist...\n";
			fs::create_directories(fullOutputPath);
		}
	}

	std::string resultingHtml;
	fullOutputPath += name;
	fullOutputPath += ".html";

	std::string templatePath = state.buildPath.data();
	templatePath += '/';
	templatePath += templ->second;
	if(!templatePath.ends_with(".html")) {
		templatePath += ".html";
	}

	if(auto it = state.templates.find(templ->second); 
			it != state.templates.end()) {
		resultingHtml = it->second.emit(*mdDoc, false);
	} else {
		auto templateSrc = consumeFile(templatePath.c_str());
		auto newTemplate = html::compile(templateSrc);
		resultingHtml = newTemplate.emit(*mdDoc, false);
		state.templates.emplace(templ->second, newTemplate);
	}

	if(verboose) {
		std::cerr << "Writing to " << fullOutputPath << '\n';
	}
	std::ofstream output(fullOutputPath);
	output << resultingHtml;
	output.close();
}
