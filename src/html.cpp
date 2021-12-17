#include "html.hpp"
#include <algorithm>

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
