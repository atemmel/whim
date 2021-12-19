#pragma once

#include "tcp_socket.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

struct Http {
	enum Method : int32_t {
		Get,
		Post,
		Delete,
		Put,
	};

	struct Message {
		std::string path;
		std::unordered_map<std::string, std::string> headers;
		Method method;

		auto setMethod(std::string_view view) -> void;
	};

	class Server {
	public:
		using Handler = std::function<void(const Message& request, TcpSocket client)>;
		auto listen() -> Result<void>;
		auto endpoint(std::string_view path, Handler callback) -> void;
	private:
		auto handleClient(TcpSocket client) -> void;
		auto parseMessage(TcpSocket client) -> Result<Message>;
		std::unordered_map<std::string, Handler> handlers;
	};
};
