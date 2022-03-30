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

		[[nodiscard]] auto emit(const md::Document& document, bool isLiveReload) -> std::string;

		auto visit(const md::Document& document) -> void;
		auto visit(const md::Paragraph& paragraph) -> void;
		auto visit(const md::Header& header) -> void;
		auto visit(const md::Code& code) -> void;
	private:
		std::string output;
		bool isLiveReload;
	};

	[[nodiscard]] static auto compile(std::string_view view) -> Template;
};
