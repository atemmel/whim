#pragma once
#include "html.hpp"
#include "markdown.hpp"
#include "result.hpp"

struct CreatingHtmlFromMarkdownState {
	std::string_view basePath;
	std::string_view markdownPath;
	std::unique_ptr<md::Document> document;
	html::Template htmlTemplate;
};

auto doLiveReload(std::string_view markdownPath) -> int;
auto createHtmlFromMarkdown(CreatingHtmlFromMarkdownState& state) -> Result<std::string>;
