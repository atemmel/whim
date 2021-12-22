#include "live_reload.hpp"

#include "utils.hpp"

#include <filesystem>

 [[nodiscard]] auto setupLiveReloadState(std::string_view markdownPath, HtmlTemplateLiveReloadState& htmlState, MarkdownLiveReloadState& markdownState) -> Result<void> {
	Result<void> result;
	markdownState.path = markdownPath;
	markdownState.htmlTemplate = &htmlState;
	markdownState.markdownSource = consumeFile(markdownPath.data());
	if(markdownState.markdownSource.empty()) {
		std::string err = "Could not read file: ";
		err += markdownPath;
		return result.set(err);
	}
	markdownState.document = md::parse(markdownState.markdownSource);
	if(!markdownState.document) {
		return result.set("Could not build markdown document");
	}

	auto templateFile = markdownState.document->meta.find("template");
	if(templateFile == markdownState.document->meta.end()) {
		return result.set("No template path specified in markdown file header");
	}

	auto markdownDir = getPath(markdownPath);
	htmlState.path = markdownDir;
	htmlState.path += std::filesystem::path::preferred_separator;
	htmlState.path += templateFile->second;

	if(!endsWith(htmlState.path, ".html")) {
		htmlState.path += ".html";
	}

	auto htmlSource = consumeFile(htmlState.path.c_str());
	if(htmlSource.empty()) {
		std::string err = "Could not read file at: ";
		err += templateFile->second;
		return result.set(err);
	}

	htmlState.htmlTemplate = html::compile(htmlSource);
	markdownState.htmlOutput = htmlState.htmlTemplate.emit(*markdownState.document);

	return result;
}

auto reloadMarkdown(MarkdownLiveReloadState& markdownState) -> void {
	auto newMarkdownSource = consumeFile(markdownState.path.c_str());
	auto newDocument = md::parse(newMarkdownSource);
	markdownState.documentMutex.lock();
	markdownState.document.swap(newDocument);
	markdownState.documentMutex.unlock();
}

auto reloadHtmlOutput(MarkdownLiveReloadState& markdownState) -> void {
	markdownState.documentMutex.lock();
	markdownState.htmlTemplate->htmlTemplateMutex.lock();
	auto newHtmlOutput = markdownState.htmlTemplate->htmlTemplate.emit(*markdownState.document);
	markdownState.htmlTemplate->htmlTemplateMutex.unlock();
	markdownState.documentMutex.unlock();

	markdownState.htmlOutputMutex.lock();
	markdownState.htmlOutput.swap(newHtmlOutput);
	markdownState.htmlOutputMutex.unlock();
}

auto reloadHtmlTemplate(MarkdownLiveReloadState& markdownState) -> void {
	auto htmlSrc = consumeFile(markdownState.htmlTemplate->path.c_str());
	auto reloadedTemplate = html::compile(htmlSrc);
	markdownState.htmlTemplate->htmlTemplateMutex.lock();
	markdownState.htmlTemplate->htmlTemplate = std::move(reloadedTemplate);
	markdownState.htmlTemplate->htmlTemplateMutex.unlock();
}
