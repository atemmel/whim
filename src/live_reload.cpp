#include "live_reload.hpp"

#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "markdown.hpp"
#include "utils.hpp"
#include "websocket.hpp"

#include <iostream>
#include <filesystem>

auto doLiveReload(std::string_view markdownPath) -> int {

	ws::Server wsServer;
	wsServer.listen(3501);

	FilesystemWatcher mdWatcher;
	auto result = mdWatcher.watch(markdownPath, [&](){
		wsServer.sendToAll("reload");
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	Http::Server server;
	std::string_view header = "HTTP/1.1 200 OK\r\n\r\n";

	auto root = [&](const Http::Message& message, TcpSocket client) {
		client.write(header);
		client.write("Welcome to root :)\n");
	};

	auto fallback = [&](const Http::Message& message, TcpSocket client) {
		std::string fullPathNoStem;
		fullPathNoStem += markdownPath;
		fullPathNoStem += '/';
		fullPathNoStem += message.path;

		if(std::filesystem::exists(fullPathNoStem) && std::filesystem::is_regular_file(fullPathNoStem)) {
			auto contents = consumeFile(fullPathNoStem.c_str());
			client.write(header);
			client.write(contents);
			client.write("\n\n");
			return;
		}

		auto mdRequest = fullPathNoStem + ".md";
		if(std::filesystem::exists(mdRequest) && std::filesystem::is_regular_file(mdRequest)) {
			CreatingHtmlFromMarkdownState state;
			state.basePath = markdownPath;
			state.markdownPath = mdRequest;
			auto response = createHtmlFromMarkdown(state);
			if(response.fail()) {
				std::cerr << response.reason();
				//TODO: write to client?
				return;
			}

			client.write(header);
			client.write(response.value);
		} else {
#ifdef DEBUG
			std::cerr << "404 moment\n";
			std::cerr << message.path << '\n';
#endif
			return;
		}
	};

	server.endpoint("/", root);
	server.fallbackEndpoint(fallback);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}

	return EXIT_SUCCESS;
}

auto createHtmlFromMarkdown(CreatingHtmlFromMarkdownState& state) -> Result<std::string> {
	Result<std::string> result;
	auto markdownSource = consumeFile(state.markdownPath.data());
	if(markdownSource.empty()) {
		std::string err = "Could not read file: ";
		err += state.markdownPath;
		return result.set(err);
	}

	state.document = md::parse(markdownSource);
	if(!state.document) {
		return result.set("Could not build markdown document");
	}

	auto templateFile = state.document->meta.find("template");
	if(templateFile == state.document->meta.end()) {
		return result.set("No template path specified in markdown file header");
	}

	std::string templatePath = state.basePath.data();
	templatePath += '/';
	templatePath += templateFile->second;
	if(!templatePath.ends_with(".html")) {
		templatePath += ".html";
	}

	auto htmlTemplateSrc = consumeFile(templatePath.c_str());
	state.htmlTemplate = html::compile(htmlTemplateSrc);
	result.value = state.htmlTemplate.emit(*state.document, true);
	return result;
}
