#include "base64.hpp"

namespace base64 {

constexpr std::string_view table =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/=";

std::string encode(std::string_view view) {
	std::string result;
	result.reserve(view.size() * 2);

	// 8 * 3 = 24
	// 24 % 6 == 0
	for(size_t step = 0; step < view.size(); step += 3) {
		char c0, c1, c2;

		c0 = (view[step] & 0xFC) >> 2;

		if(view.size() <= step + 1) {
			c1 = 64 + (view[step] & 0b11000000);
			c2 = 64;
		} else {
			c1 = (view[step + 1] & 0xFC) >> 2;
			if(view.size() <= step + 2) {
				c2 = 64 + (view[step + 1] & 0b11000000);
			} else {
				c2 = (view[step + 2] & 0xFC) >> 2;
			}
		}


		result.push_back(table[c0]);
		result.push_back(table[c1]);
		result.push_back(table[c2]);
	}

	return result;
}

};
