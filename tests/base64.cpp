#include "catch.hpp"

#include "base64.hpp"

TEST_CASE("base64 encoding", "[base64::encode]") {
	SECTION("Validity") {
		REQUIRE(base64::encode("light work.") == "bGlnaHQgd29yay4=");
		REQUIRE(base64::encode("light work") == "bGlnaHQgd29yaw==");
		REQUIRE(base64::encode("light wor") == "bGlnaHQgd29y");
	}
}
