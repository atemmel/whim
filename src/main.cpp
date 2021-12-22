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

void handleWs(TcpSocket client) {
	auto result = Http::parseMessage(client);
	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return;
	}

	/*
	for(auto& it : result.value.headers) {
		std::cerr << it.first << ' ' << it.second << '\n';
	}
	*/

	auto it = result.value.headers.find("Sec-WebSocket-Key");
	if(it == result.value.headers.end()) {
		std::cerr << "Nu-uh\n";
		return;
	}

	const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	std::string acceptUnhashed = it->second + magic;
	auto hash = sha1::hash(acceptUnhashed);
	auto hashAsView = std::string_view(reinterpret_cast<const char*>(hash.data()), hash.size());
	auto base64Hash = base64::encode(hashAsView);

	std::string response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n";
	response += "Sec-Websocket-Accept: " + base64Hash + "\n\n";
	client.write(response);

	auto decodeFrame = ws::decode(client);

	if(decodeFrame.fail()) {
		std::cerr << decodeFrame.reason() << '\n';
		return;
	}

	auto frame = std::move(decodeFrame.value);
	std::cerr << (int)frame.op << '\n';
}

void ws() {
	ws::Server server;
	server.listen(3501);
}

auto main(int argc, char** argv) -> int {

	//std::thread other(ws);
	//other.detach();

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

	ws::Server wsServer;
	wsServer.listen(3501);

	FilesystemWatcher mdWatcher;
	auto result = mdWatcher.watch(markdownState.path, [&](){
		std::cerr << "Changed markdown!\n";
		reloadMarkdown(markdownState);
		reloadHtmlOutput(markdownState);
		wsServer.sendToAll("reload");
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
