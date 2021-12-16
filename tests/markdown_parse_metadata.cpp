#include "catch.hpp"

#include "markdown.hpp"

TEST_CASE("Parsing markdown metadata", "[md::parse]") {
	std::string_view src = R"(
---
key   = value    
title = Cool title
------
other cool text in here lmao
)";

	auto doc = md::parse(src);

	SECTION("Validity") {
		REQUIRE(doc);
		auto it = doc->meta.find("key");
		REQUIRE(it != doc->meta.end());
		REQUIRE(it->second == "value");
		it = doc->meta.find("title");
		REQUIRE(it != doc->meta.end());
		REQUIRE(it->second == "Cool title");
	}
}

TEST_CASE("Parse text block", "[md::parse]") {
	std::string_view src = R"(
lorem ipsum dolor sit amet
lorem ipsum dolor sit amet
lorem ipsum dolor sit amet

lorem ipsum dolor sit amet

lorem ipsum dolor sit amet
lorem ipsum dolor sit amet
)";

	auto doc = md::parse(src);

	SECTION("Validity") {
		REQUIRE(doc);
		REQUIRE(doc->children.size() == 3);
		auto p1 = dynamic_cast<md::Paragraph*>(doc->children[0].get());
		REQUIRE(p1);
		REQUIRE(p1->contents == R"(lorem ipsum dolor sit amet
lorem ipsum dolor sit amet
lorem ipsum dolor sit amet)");
		auto p2 = dynamic_cast<md::Paragraph*>(doc->children[1].get());
		REQUIRE(p2);
		REQUIRE(p2->contents == "lorem ipsum dolor sit amet");
		auto p3 = dynamic_cast<md::Paragraph*>(doc->children[2].get());
		REQUIRE(p3);
		REQUIRE(p3->contents == R"(lorem ipsum dolor sit amet
lorem ipsum dolor sit amet)");
	}
}

TEST_CASE("Parsing markdown header", "[md::parse]") {
	std::string_view src = R"(
# this is a header of depth 1
this is not a header
### this is a header of depth 3
####### this is not a header
#this is not a header
)";

	auto doc = md::parse(src);

	//md::Printer printer;
	//printer.visit(*doc);

	SECTION("Validity") {
		REQUIRE(doc);
		REQUIRE(doc->children.size() == 5);
		auto h1 = dynamic_cast<md::Header*>(doc->children[0].get());
		REQUIRE(h1);
		REQUIRE(h1->contents == "this is a header of depth 1");
		REQUIRE(h1->level == 1);
		auto p1 = dynamic_cast<md::Paragraph*>(doc->children[1].get());
		REQUIRE(p1);
		REQUIRE(p1->contents == "this is not a header");
		auto h2 = dynamic_cast<md::Header*>(doc->children[2].get());
		REQUIRE(h2);
		REQUIRE(h2->contents == "this is a header of depth 3");
		REQUIRE(h2->level == 3);
		auto p2 = dynamic_cast<md::Paragraph*>(doc->children[3].get());
		REQUIRE(p2);
		REQUIRE(p2->contents == "####### this is not a header");
		auto p3 = dynamic_cast<md::Paragraph*>(doc->children[4].get());
		REQUIRE(p3);
		REQUIRE(p3->contents == "#this is not a header");
	}
}
