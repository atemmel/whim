#pragma once
#include "error.hpp"

struct TcpSocket {
	static auto create() -> TcpSocket;
	
	auto listen(uint16_t port) -> void;
	auto accept() -> TcpSocket;

	auto read(size_t howManyBytes) -> std::string;
	auto readUntil(char thisByte) -> std::string;
	auto write(std::string_view view) -> size_t;
private:
	int fd;
};
