#pragma once

#include <string>
#include <string_view>
#include <vector>

struct html {
	struct Template {
		struct InsertionPoint {
			std::string_view contents;
			size_t first, last;
		};

		std::vector<InsertionPoint> insertionPoints;
		std::string contents;
	};

	[[nodiscard]] static auto compile(std::string_view view) -> Template;
};
