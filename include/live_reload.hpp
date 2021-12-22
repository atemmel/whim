#pragma once
#include "html.hpp"
#include "markdown.hpp"
#include "result.hpp"

#include <thread>

struct HtmlTemplateLiveReloadState {
	html::Template htmlTemplate;
	std::mutex htmlTemplateMutex;
	std::string path;
};

struct MarkdownLiveReloadState {
	std::string markdownSource;
	std::unique_ptr<md::Document> document;
	std::mutex documentMutex;

	std::string htmlOutput;
	std::mutex htmlOutputMutex;
	std::string path;

	HtmlTemplateLiveReloadState* htmlTemplate;
};


 [[nodiscard]] auto setupLiveReloadState(std::string_view markdownPath, HtmlTemplateLiveReloadState& htmlState, MarkdownLiveReloadState& markdownState) -> Result<void>;

auto reloadMarkdown(MarkdownLiveReloadState& markdownState) -> void;
auto reloadHtmlOutput(MarkdownLiveReloadState& markdownState) -> void;
auto reloadHtmlTemplate(MarkdownLiveReloadState& markdownState) -> void;
