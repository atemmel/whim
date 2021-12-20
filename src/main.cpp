#include "argparser.hpp"
#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "live_reload.hpp"
#include "markdown.hpp"

#include <iostream>

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
