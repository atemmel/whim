#include "argparser.hpp"
#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "live_reload.hpp"
#include "markdown.hpp"

#include <iostream>

void handleWs(TcpSocket client) {
	auto result = client.read(1024);
	if(result.success()) {
		std::cerr << result.value << '\n';
	} else {
		std::cerr << result.reason() << '\n';
	}
}

void ws() {
	Result<void> toReturn;
	auto result = TcpSocket::create();
	
	if(result.fail()) {
		return;
	}

	auto socket = result.value;
	auto voidRes = socket.listen(3501);
	if(voidRes.fail()) {
		return;
	}

	while(true) {
		auto client = socket.accept();
		if(client.fail()) {
			std::cerr << client.reason() << '\n';
		} else {
			handleWs(client.value);
		}
	}
}

auto main(int argc, char** argv) -> int {

	std::thread other(ws);
	other.detach();

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
		std::cerr << "Sending\n";
		std::cerr << markdownState.htmlOutput << '\n';
		markdownState.htmlOutputMutex.unlock();
	};

	server.endpoint("/", root);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}
}
