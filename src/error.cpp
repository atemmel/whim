#include "error.hpp"
#include <string_view>

thread_local std::string Error::error;

auto Error::yes() -> bool {
	return !no();
}

auto Error::no() -> bool {
	return error.empty();
}

auto Error::view() -> const std::string_view {
	return std::string_view(error);
}

auto Error::set(std::string &&str) -> void {
	error = str;
}
