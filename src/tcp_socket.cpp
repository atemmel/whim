#include "tcp_socket.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

auto TcpSocket::create() -> TcpSocket {
	TcpSocket tcpSocket;
	tcpSocket.fd = socket(AF_INET, SOCK_STREAM, 0);
	/*	Error handling
	if(tcpSocket.fd < 0) {

	}
	*/

	return tcpSocket;
}

auto TcpSocket::listen(uint16_t port) -> void {
	
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		//return "Failed to set SO_REUSEADDR option";
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		//return "Error setting SO_REUSEPORT option";
	}

	int result = ::bind(fd, reinterpret_cast<const sockaddr*>(&hint), sizeof hint);
	if(result != 0) {
		//return "Error binding port";
	}

	result = ::listen(fd, 256);

	if(result != 0) {
		//return "Could not set socket into listening state";
	}

	//return nullptr;
}

auto TcpSocket::accept() -> TcpSocket {

	TcpSocket client;
	client.fd = ::accept(fd, nullptr, nullptr);
	if(client.fd == -1) {
		//"Failed to accept incoming connection",
	}

	return client;
}

auto TcpSocket::read(size_t howManyBytes) -> std::string {
	std::string bytes(howManyBytes, '\0');
	auto result = ::read(fd, bytes.data(), bytes.size());
	if(result < 0) {
		/*
		return {
			{},
			"Reading from socket failed",
		};
		*/
	}
	return bytes;
}

auto TcpSocket::readUntil(char thisByte) -> std::string {
	std::string bytes;
	bytes.reserve(64);

	char byte = 0;
	while(true) {
		auto result = ::read(fd, &byte, 1);
		if(result < 0) {
			/*
			return {
				{},
				"Reading from socket failed",
			};
			*/
		}

		bytes.push_back(byte);

		if(byte == thisByte) {
			return bytes;
		}
	}
}

auto TcpSocket::write(std::string_view bytes) -> size_t {
	auto result = ::write(fd, bytes.data(), bytes.size());
	if(result < 0) {
		/*
		return {
			0,
			"Writing to socket failed",
		};
		*/
	}
	return result;
}

