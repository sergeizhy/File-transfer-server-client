#pragma once
#include "Socket.h"

Socket::Socket() :sock(nullptr), reslv(nullptr) ,connected(SOCK::DISCONNECTED){}

Socket::~Socket() {
	this->disconnect();
	delete sock;
	delete reslv;
}

bool l_endians() {
	uint c = 0xccccffff;
	if ((byte)c == 0xcc)
		throw std::exception("the system isnt using little endians");
	return true;
}

SOCK Socket::connect(const std::string host, const std::string port) {
	try {
		reslv = new tcp::resolver(context);
		sock = new tcp::socket(context);
		if (l_endians()) {
			asio::connect(*sock, reslv->resolve(host, port));
			sock->non_blocking(false);
			connected = SOCK::CONNECTED;
		}
	}
	catch (const system::system_error& error) {
		printf(error.what());
		return SOCK::ERR_CONNECT;
	}
	catch (const std::exception& error){
		printf(error.what());
		return SOCK::ERR_CONNECT;
	}
	return SOCK::CONNECTED;
}

SOCK Socket::disconnect() {
	try {
		sock->close();
		connected = SOCK::DISCONNECTED;
	}
	catch (const system::system_error& err) {
		printf(err.what());
		return SOCK::ERR_DISCONNECT;
	}
	return SOCK::DISCONNECTED;
}

std::vector<std::pair<byte*, size_t>> split(std::pair<byte*, size_t> buffer)
{
	std::vector<std::pair<byte*, size_t>> fragments;
	byte* fragment = nullptr;
	size_t size = max_buffer;

	for (size_t off = 0; off < buffer.second; off += max_buffer)
	{
		if ((buffer.second - off) < max_buffer)
			size = buffer.second - off;
		fragment = new byte[size];

		memset(fragment, NULL, size);
		memcpy_s(fragment, size, buffer.first + off, size);

		fragments.push_back({ fragment,size });
	}
	delete[] buffer.first;

	return fragments;
}

SOCK Socket::send(Request& request) {
	
	std::vector<std::pair<unsigned char*,size_t>> data_fragments = split(request.pack());
	try 
	{
		printf("client sent %d\n", request.header.code);
		for (std::pair<unsigned char*,size_t> fragment: data_fragments) {
			asio::write(*sock, asio::buffer(fragment.first, fragment.second));
			delete[] fragment.first;
		}
	}
	catch (const system::system_error& err) {
		printf(err.what());
		return SOCK::ERR_SEND;
	}
	return SOCK::SENT;
}

SOCK Socket::receive(Response& response) {
	size_t packet_size = sizeof(response.header);
	size_t received_size = 0;
	uint payload_size = 0;
	byte fragment[max_buffer] = {0};
	bool header = true;

	try {
		response.clear();
		while (received_size < packet_size) {
			received_size += asio::read(*sock, asio::buffer(fragment + received_size, packet_size - received_size));
			if (header) {
				memcpy(&payload_size, fragment + 3, sizeof(uint));
				packet_size += payload_size;
				header = !header;
			}
		}
		response.unpack(fragment, packet_size);
		printf("client received %d\n", response.header.code);

	}	
	catch (const system::system_error& err) {
		printf(err.what());
		return SOCK::ERR_RECEIVE;
	}
	return SOCK::RECEIVED;
}
