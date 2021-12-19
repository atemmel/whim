#pragma once
#include "result.hpp"

struct TcpSocket {
	[[nodiscard]] static auto create() -> Result<TcpSocket>;
	
	auto close() const -> void;
	auto listen(uint16_t port) const -> Result<void>;
	auto accept() const -> Result<TcpSocket>;

	auto read(size_t howManyBytes) const -> Result<std::string>;
	auto readUntil(char thisByte) const -> Result<std::string>;
	auto write(std::string_view view) const -> Result<size_t>;
private:
	int fd;
};
