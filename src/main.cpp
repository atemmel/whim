#include "html.hpp"
#include "http.hpp"
#include "markdown.hpp"

#include <iostream>

auto main() -> int {
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

	//TODO: Implement next step in pipeline
	//auto doc = md::parse(markdown);
	//auto htmlTemplate = html::compile(html);

	auto root = [&](const Http::Message& message, TcpSocket client) {
		std::string_view header = "HTTP/1.1 200 OK\n\n";
		std::string_view response = html;
		client.write(header);
		client.write(response);
	};

	server.endpoint("/", root);

	auto res = server.listen();
	if(res.fail()) {
		std::cerr << res.reason() << '\n';
	}
}
