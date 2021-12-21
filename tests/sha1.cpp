#include "catch.hpp"

#include "sha1.hpp"

TEST_CASE("SHA1 hash", "[sha1::hash]") {
	SECTION("Validity") {
		sha1::Hash target = {
			0x2f, 0xd4, 0xe1, 0xc6, 0x7a, 0x2d, 0x28, 0xfc, 0xed, 0x84, 
			0x9e, 0xe1, 0xbb, 0x76, 0xe7, 0x39, 0x1b, 0x93, 0xeb, 0x12,
		};
		auto computed = sha1::hash("The quick brown fox jumps over the lazy dog");
		REQUIRE(target == computed);

		target = {
			0xde, 0x9f, 0x2c, 0x7f, 0xd2, 0x5e, 0x1b, 0x3a, 0xfa, 0xd3,
			0xe8, 0x5a, 0x0b, 0xd1, 0x7d, 0x9b , 0x10, 0x0d, 0xb4, 0xb3,
		};
		computed = sha1::hash("The quick brown fox jumps over the lazy cog");
		REQUIRE(target == computed);
	}
}