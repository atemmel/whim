#include "sha1.hpp"

namespace sha1 {

constexpr uint32_t h0 = 0x01234567;
constexpr uint32_t h1 = 0x89ABCDEF;
constexpr uint32_t h2 = 0xFEDCBA98;
constexpr uint32_t h3 = 0x76543210;
constexpr uint32_t h4 = 0xF0E1D2C3;

constexpr uint32_t ml = sizeof(Hash);

auto hash(std::string_view view) -> Hash {
	Hash h;

	return h;
}

};
