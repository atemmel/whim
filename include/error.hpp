#pragma once

#include <string>

class Error {
public:
	static auto yes() -> bool;
	static auto no() -> bool;
	static auto view() -> const std::string_view;
	static auto set(std::string&& str) -> void;
private:
	static thread_local std::string error;
};
