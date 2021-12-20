#include "argparser.hpp"
#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "markdown.hpp"
#include "utils.hpp"

#include <iostream>
#include <thread>

auto main(int argc, char** argv) -> int {
	std::string markdownSource;

	ArgParser argParser(argc, argv);
	argParser.addString(&markdownSource, "serve");
	auto parseResult = argParser.unwind();
	if(parseResult.fail()) {
		std::cerr << parseResult.reason();
		return EXIT_FAILURE;
	}

	std::unique_ptr<md::Document> mdDoc;
	std::mutex mdDocMutex;

	std::string htmlOutput;
	std::mutex htmlOutputMutex;

	html::Template htmlTemplate;

	auto readMd = [&]() {
		auto mdSrc = consumeFile(markdownSource.c_str());
		auto newDoc = md::parse(mdSrc);
		mdDocMutex.lock();
		newDoc.swap(mdDoc);
		mdDocMutex.unlock();
	};

	auto emitHtml = [&]() {
		mdDocMutex.lock();
		auto newOutput = htmlTemplate.emit(*mdDoc);
		mdDocMutex.unlock();
		htmlOutputMutex.lock();
		htmlOutput.swap(newOutput);
		htmlOutputMutex.unlock();
	};

	readMd();

	auto templatePath = mdDoc->meta.find("template");
	if(templatePath == mdDoc->meta.end()) {
		std::cerr << "No template path specified in markdown file header\n";
		return EXIT_FAILURE;
	}

	auto markdownSourcePath = getPath(markdownSource);
	auto fullTemplatePath = std::string(markdownSourcePath);
	fullTemplatePath += std::filesystem::path::preferred_separator;
	fullTemplatePath += templatePath->second;

	if(!endsWith(fullTemplatePath, ".html")) {
		fullTemplatePath += ".html";
	}

	auto htmlSource = consumeFile(fullTemplatePath.c_str());
	if(htmlSource.empty()) {
		std::cerr << "Could not read file at: " << templatePath->second << '\n';
		return EXIT_FAILURE;
	}
	htmlTemplate = html::compile(htmlSource);
	emitHtml();

	FilesystemWatcher mdWatcher;
	auto result = mdWatcher.watch(markdownSource, [&](){
		std::cerr << "Changed markdown!\n";
		readMd();
		emitHtml();
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	FilesystemWatcher htmlWatcher;
	result = htmlWatcher.watch(fullTemplatePath, []() {
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
		htmlOutputMutex.lock();
		client.write(htmlOutput);
		htmlOutputMutex.unlock();
	};

	server.endpoint("/", root);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}
}
