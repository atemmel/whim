#include "filesystem_watcher.hpp"

#include <iostream>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

auto FilesystemWatcher::watch(std::string_view path, Callback callback, long msDelta) -> Result<void> {
	watched = path;
	toCall = callback;
	sleepFor = msDelta;
	Result<void> result;

	auto timestamp = getTimestamp();
	if(timestamp.fail()) {
		return result.set(timestamp.reason());
	}

	lastWritten = timestamp.value;

	std::thread watcher([&]() {
		watch();
	});
	watcher.detach();

	return result;
}

auto FilesystemWatcher::watch() -> void {
REDO:
	auto timestamp = getTimestamp();
	if(timestamp.success() && timestamp.value > lastWritten) {
		lastWritten = timestamp.value;
		toCall();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));
	goto REDO;
}

auto FilesystemWatcher::getTimestamp() const -> Result<long> {
	Result<long> result;

	if(!fs::exists(watched)) {
		std::string err = "Could not find a file named: ";
		err += watched;
		return result.set(err);
	}

	if(!fs::is_regular_file(watched)) {
		std::string err = "Could not listen to changes in: ";
		err += watched;
		err += ", as it is not a regular file";
		return result.set(err);
	}

	auto time = fs::last_write_time(watched);
	result.value = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::file_clock::to_sys(time).time_since_epoch()).count();
	return result;
};
