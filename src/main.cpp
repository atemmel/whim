#include "argparser.hpp"
#include "base64.hpp"
#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "live_reload.hpp"
#include "markdown.hpp"
#include "sha1.hpp"
#include "websocket.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int {

	std::string markdownPath;

	ArgParser argParser(argc, argv);
	argParser.addString(&markdownPath, "serve", "Serve a file for real-time editing");
	auto parseResult = argParser.unwind();
	if(parseResult.fail()) {
		std::cerr << parseResult.reason();
		return EXIT_FAILURE;
	}

	if(markdownPath.empty()) {
		argParser.usage();
	};

	HtmlTemplateLiveReloadState htmlState;
	MarkdownLiveReloadState markdownState;

	auto setupResult = setupLiveReloadState(markdownPath, htmlState, markdownState);
	if(setupResult.fail()) {
		std::cerr << setupResult.reason() << '\n';
		return EXIT_FAILURE;
	}

	ws::Server wsServer;
	wsServer.listen(3501);

	FilesystemWatcher mdWatcher;
	auto result = mdWatcher.watch(markdownState.path, [&](){
		reloadMarkdown(markdownState);
		reloadHtmlOutput(markdownState);
		wsServer.sendToAll("reload");
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	FilesystemWatcher htmlWatcher;
	result = htmlWatcher.watch(htmlState.path, [&]() {
		reloadHtmlTemplate(markdownState);
		reloadMarkdown(markdownState);
		reloadHtmlOutput(markdownState);
		wsServer.sendToAll("reload");
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
