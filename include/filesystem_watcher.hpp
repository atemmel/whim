#pragma once
#include "result.hpp"

#include <filesystem>
#include <functional>
#include <string_view>


class FilesystemWatcher {
public:
	using Callback = std::function<void()>;
	[[nodiscard]] auto watch(std::string_view path, Callback callback, long msDelta) -> Result<void>;
private:
	auto watch() -> void;
	auto getTimestamp() const -> Result<long>;
	std::filesystem::path watched;
	Callback toCall;
	long lastWritten;
	long sleepFor;
};
