#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <unordered_map>
#include "Cryptowrapper.h"
#include "Packet.h"
#include <cstdint>
#include <string>
#include <vector>


using namespace	boost;
using asio::ip::tcp;

constexpr auto			max_buffer = 0x400;

enum class SOCK {
	DISCONNECTED = 0,
	CONNECTED,
	ERR_CONNECT,
	ERR_DISCONNECT,
	UNK_ERR,
	ERR_RECEIVE,
	ERR_SEND,
	RECEIVED,
	SENT
};

class Socket {
private:
	
	tcp::socket*		sock;
	tcp::resolver*		reslv;
	asio::io_context	context;
	SOCK				connected;
public:
	
	Socket();
	virtual ~Socket();
	SOCK connect(const std::string host,const std::string port);
	SOCK disconnect();
	SOCK receive(Response& res);
	SOCK send(Request& req);
};

