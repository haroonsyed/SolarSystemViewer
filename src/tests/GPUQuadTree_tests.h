#pragma once
#include <catch2/catch.hpp>

TEST_CASE("Test Test Pass") {
	REQUIRE(1 == 1);
}

TEST_CASE("Test Test Fail") {
	REQUIRE(1 == 1);
}