#include "html.hpp"

#include <algorithm>
#include <iostream>

auto html::Template::emit(const md::Document& document, bool isLiveReload) -> std::string {
	output.clear();
	this->isLiveReload = isLiveReload;
	visit(document);
	return output;
}

auto html::Template::visit(const md::Document& document) -> void {
	output.reserve(contents.size());
	auto prevIndex = 0;
	for(auto point : insertionPoints) {
		output += std::string_view(contents.begin() + prevIndex,
				contents.begin() + point.first - 1);
		auto meta = document.meta.find(std::string(point.contents));
		if(meta != document.meta.end()) {
			output += meta->second;
		} else if(point.contents == "main") {
			for(const auto& child : document.children) {
				child->accept(*this);
			}
		}

		prevIndex = point.last;
	}

	std::string_view endBodyStr = "</body>";
	auto bodyEnd = std::find_end(contents.begin() + prevIndex, contents.end(),
			endBodyStr.begin(), endBodyStr.end());

	output += std::string_view(contents.begin() + prevIndex, bodyEnd);

	if(isLiveReload) {
		output += R"(
		 <script type="text/javascript">
function connect() {
	var ws = new WebSocket('ws://localhost:3501');

	ws.onopen = function(err) {
		console.log("Connection to whim established");
	};

	ws.onmessage = function(e) {
		if(e.data === 'reload') {
			location.reload();
		}
	};

	ws.onclose = function(e) {
		setTimeout(function() {
		connect();
		}, 1000);
	};

	ws.onerror = function(err) {
		console.error('Socket encountered error: ', err.message, 'Closing socket');
		ws.close();
	};
}
connect();
		</script>

)";
	}

	output += std::string_view(bodyEnd, contents.end());
}

auto html::Template::visit(const md::Paragraph& paragraph) -> void {
	output += "<p>";
	output += paragraph.contents;
	output += "</p>";
}

auto html::Template::visit(const md::Header& header) -> void {
	switch(header.level) {
		case 1:
			output += "<h1>";
			break;
		case 2:
			output += "<h2>";
			break;
		case 3:
			output += "<h3>";
			break;
		case 4:
			output += "<h4>";
			break;
		case 5:
			output += "<h5>";
			break;
		case 6:
			output += "<h6>";
			break;
	}

	output += header.contents;

	switch(header.level) {
		case 1:
			output += "</h1>";
			break;
		case 2:
			output += "</h2>";
			break;
		case 3:
			output += "</h3>";
			break;
		case 4:
			output += "</h4>";
			break;
		case 5:
			output += "</h5>";
			break;
		case 6:
			output += "</h6>";
			break;
	}
}

auto html::Template::visit(const md::Code& code) -> void {
	output += "<pre><code>";
	output += code.contents;
	output += "</pre></code>";
}

auto html::compile(std::string_view view) -> html::Template {
	Template html;
	html.contents = view;

	for(auto it = std::find(view.begin(), view.end(), '$'); it != view.end(); it = std::find(it, view.end(), '$')) {
		it++;
		size_t first = std::distance(view.begin(), it);
		it = std::find_if(it, view.end(), [](char c) { 
			return !(std::isalnum(c) || c == '_'); 
		});
		size_t last = std::distance(view.begin(), it);

		std::string_view contents = std::string_view(view.begin() + first, view.begin() + last);
		html.insertionPoints.emplace_back(contents, first, last);
	}

	return html;
}
