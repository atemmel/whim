#pragma once

#include "markdown.hpp"

#include <string>
#include <string_view>
#include <vector>

struct html {
	struct Template : md::Visitor {
		struct InsertionPoint {
			std::string_view contents;
			size_t first, last;
		};

		std::vector<InsertionPoint> insertionPoints;
		std::string contents;

		[[nodiscard]] auto emit(const md::Document& document) -> std::string;

		auto visit(const md::Document& document) -> void;
		auto visit(const md::Paragraph& paragraph) -> void;
		auto visit(const md::Header& header) -> void;
	private:
		std::string output;
	};

	[[nodiscard]] static auto compile(std::string_view view) -> Template;
};
