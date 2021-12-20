#include "html.hpp"
#include "http.hpp"
#include "filesystem_watcher.hpp"
#include "markdown.hpp"

#include <iostream>

auto main() -> int {

	FilesystemWatcher watcher;
	auto result = watcher.watch("../compile_commands.json", [](){
		std::cerr << "Changed!\n";
	}, 200);

	if(result.fail()) {
		std::cerr << result.reason() << '\n';
		return EXIT_FAILURE;
	}

	Http::Server server;

	std::string_view markdown = R"(
---
key = value
title = cool title
------
epic content moment
)";

	std::string_view html = R"(
<!DOCTYPE HTML>
<html>
	<head>
		<meta charset="UTF-8">
		<title>$title</title>
	</head>
	<body>
		<h3>$title</h3>
		$main
	</body>
</html>
)";

	auto doc = md::parse(markdown);
	auto htmlTemplate = html::compile(html);
	auto output = htmlTemplate.emit(*doc);

	auto root = [&](const Http::Message& message, TcpSocket client) {
		std::string_view header = "HTTP/1.1 200 OK\n\n";
		std::string_view response = html;
		client.write(header);
		client.write(output);
	};

	server.endpoint("/", root);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}
}
