#pragma once
#include <iostream>
#include <unordered_map>
#include "Streamhandler.h"
#include "Socket.h"

class SocketHandler {
	typedef void	(SocketHandler::* request_function)	(Request& req);
	typedef ushort	(SocketHandler::* response_function)(Response& res);

	std::unordered_map<ushort,response_function>	response_map;
	std::unordered_map<ushort,request_function>		request_map;

	Symmetric					symmetric;
	Asymmetric					asymmetric;
	Socket						client;
	uint						attempts;
	std::vector<std::string>	file;

	std::string					name,
								crc,
								uuid,
								host,
								port,
								file_path;

	ushort initialize_communication(Request& request);

	void register_user(Request&);					//1100
	void request_aes_key(Request&);					//1101
	void pack_file(Request&);						//1103
	void checksum_auth(Request&);					//1104
	void checksum_auth_failed(Request&);			//1105
	void failed_to_transmit_file(Request&);			//1106
	ushort user_registered(Response&);				//2100
	ushort user_registration_failed(Response&);		//2101
	ushort response_aes_key(Response&);				//2102
	ushort checksum(Response&);						//2103
	
public:
	SocketHandler();
	~SocketHandler();
	void initialize();
	

};	