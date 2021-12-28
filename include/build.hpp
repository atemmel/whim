#pragma once

#include "html.hpp"

#include <string_view>

struct BuildProjectState {
	std::unordered_map<std::string, html::Template> templates;
	std::vector<std::string> targets;
	std::string_view buildPath;
	std::string_view outputPath;
};

auto doBuildProject(std::string_view buildPath, std::string_view outputPath, bool verboose) -> int;

auto buildFile(BuildProjectState& state, std::string_view file, bool verboose) -> void;
