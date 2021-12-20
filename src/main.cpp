#include "argparser.hpp"
#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "markdown.hpp"
#include "utils.hpp"

#include <iostream>
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

auto main(int argc, char** argv) -> int {
	std::string markdownPath;

	ArgParser argParser(argc, argv);
	argParser.addString(&markdownPath, "serve");
	auto parseResult = argParser.unwind();
	if(parseResult.fail()) {
		std::cerr << parseResult.reason();
		return EXIT_FAILURE;
	}

	HtmlTemplateLiveReloadState htmlState;
	MarkdownLiveReloadState markdownState;

	auto setupResult = setupLiveReloadState(markdownPath, htmlState, markdownState);
	if(setupResult.fail()) {
		std::cerr << setupResult.reason() << '\n';
		return EXIT_FAILURE;
	}

	FilesystemWatcher mdWatcher;
	auto result = mdWatcher.watch(markdownState.path, [&](){
		std::cerr << "Changed markdown!\n";
		reloadMarkdown(markdownState);
		reloadHtmlOutput(markdownState);
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	FilesystemWatcher htmlWatcher;
	result = htmlWatcher.watch(htmlState.path, []() {
		std::cerr << "Changed html!\n";
		std::cerr << "This does not do anything at the moment...\n";
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	Http::Server server;

	auto root = [&](const Http::Message& message, TcpSocket client) {
		std::string_view header = "HTTP/1.1 200 OK\r\n\r\n";
		client.write(header);
		markdownState.htmlOutputMutex.lock();
		client.write(markdownState.htmlOutput);
		markdownState.htmlOutputMutex.unlock();
	};

	server.endpoint("/", root);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}
}
