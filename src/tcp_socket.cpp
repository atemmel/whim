#include "tcp_socket.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

auto TcpSocket::create() -> Result<TcpSocket> {
	//TcpSocket tcpSocket;
	Result<TcpSocket> tcpSocket;
	tcpSocket.value.fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcpSocket.value.fd < 0) {
		tcpSocket.set("Could not create socket");
	}

	return tcpSocket;
}

auto TcpSocket::close() const -> void {
	::close(fd);
}

auto TcpSocket::listen(uint16_t port) const -> Result<void> {
	Result<void> result;
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		return result.set("Failed to set SO_REUSEADDR option");
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		return result.set("Failed to set SO_REUSEPORT option");
	}

	int code = ::bind(fd, reinterpret_cast<const sockaddr*>(&hint), sizeof hint);
	if(code != 0) {
		return result.set("Error binding port");
	}

	code = ::listen(fd, 256);

	if(code != 0) {
		return result.set("Could not set socket into listening state");
	}

	return result;
}

auto TcpSocket::accept() const -> Result<TcpSocket> {

	Result<TcpSocket> client;
	client.value.fd = ::accept(fd, nullptr, nullptr);
	if(client.value.fd == -1) {
		return client.set("Failed to accept incoming connection");
	}

	return client;
}

auto TcpSocket::read(size_t howManyBytes) const -> Result<std::string> {
	Result<std::string> result;
	result.value.resize(howManyBytes, '\0');
	auto code = ::read(fd, result.value.data(), result.value.size());
	if(code < 0) {
		return result.set("Reading from socket failed");
	}
	return result;
}

auto TcpSocket::readUntil(char thisByte) const -> Result<std::string> {
	Result<std::string> result;
	result.value.reserve(64);

	char byte = 0;
	while(true) {
		auto code = ::read(fd, &byte, 1);
		if(code < 0) {
			return result.set("Reading from socket failed");
		}

		if(byte == thisByte) {
			return result;
		}

		result.value.push_back(byte);
	}
}

auto TcpSocket::readBytes(size_t howManyBytes) const -> Result<std::vector<Byte>> {
	Result<std::vector<Byte>> result;
	result.value.resize(howManyBytes);
	auto code = ::read(fd, result.value.data(), result.value.size());
	if(code < 0) {
		return result.set("Reading from socket failed");
	}
	return result;
}

auto TcpSocket::write(std::string_view bytes) const -> Result<size_t> {
	Result<size_t> result;
	auto code = ::write(fd, bytes.data(), bytes.size());
	if(code < 0) {
		return result.set("Writing to socket failed");
	}
	result.value = code;
	return result;
}


auto TcpSocket::write(const std::vector<Byte>& bytes) const -> Result<size_t> {
	Result<size_t> result;
	auto code = ::write(fd, bytes.data(), bytes.size());
	if(code < 0) {
		return result.set("Writing to socket failed");
	}
	result.value = code;
	return result;
}

auto TcpSocket::operator<(TcpSocket rhs) const -> bool {
	return fd < rhs.fd;
}

