#include "SocketHandler.h"

SocketHandler::SocketHandler():host(""),port(""),file_path(""),crc("") ,attempts(3){
	uuid.resize(uuid_size);
	name.resize(name_size);
	response_map = {
		{RS2100, &SocketHandler::user_registered},
		{RS2101, &SocketHandler::user_registration_failed},
		{RS2102, &SocketHandler::response_aes_key},
		{RS2103, &SocketHandler::checksum},
	};
	request_map = {
		{RQ1100, &SocketHandler::register_user},
		{RQ1101, &SocketHandler::request_aes_key},
		{RQ1103, &SocketHandler::pack_file},
		{RQ1104, &SocketHandler::checksum_auth},
		{RQ1105, &SocketHandler::checksum_auth_failed},
		{RQ1106, &SocketHandler::failed_to_transmit_file}
	};
}
SocketHandler::~SocketHandler() {}

ushort SocketHandler::initialize_communication(Request& request) {
	file = StreamHandler::read_from_file("me.info");
	std::string key;
	if (file.size() > 2) {
		name = file[0];
		uuid = StreamHandler::decode_hex(file[1]);
		file.erase(file.begin() + 2,file.end());
		return RQ1101;
	}
	file.clear();
	return RQ1100;
}
void SocketHandler::register_user(Request& request) {
	request_1100 payload;
	size_t payload_size = 0;
	if (name.size() > 0 ) {
		file.push_back(name);
		payload_size = payload.load(name);
		request.load(uuid, RQ1100, &payload, payload_size);
	}
}

ushort SocketHandler::user_registered(Response& response) {
	std::string encoded_hex;
	uuid = std::string((char*)response.response_2100.uuid, uuid_size);
	encoded_hex = StreamHandler::encode_hex(uuid);
	file.push_back(encoded_hex);
	return RQ1101;
}

ushort SocketHandler::user_registration_failed(Response&) {
	printf("user registration failed\n");
	return 0;
}

void SocketHandler::request_aes_key(Request& request) {
	std::pair<std::string, std::string> keys = asymmetric.generate_random_keys();
	std::string private_key_base64;
	request_1101 payload;
	size_t payload_size;

	payload_size = payload.load(name, keys.first);
	request.load(uuid, RQ1101, &payload, payload_size);
	private_key_base64 = StreamHandler::encode_base_64(keys.second);
	file.push_back(private_key_base64);
	StreamHandler::write_to_file("me.info",file,0);
}

ushort SocketHandler::response_aes_key(Response& response) {
	std::string decrypted;
	decrypted = asymmetric.decrypt(response.response_2102.aes_key, Asymmetric::key_size);
	symmetric.load_key(decrypted);
	return RQ1103;
}

void SocketHandler::pack_file(Request& request) {
	std::pair<byte*, size_t> content;
	std::pair<byte*,size_t> enc_content;
	std::string file_name;
	request_1103 payload;
	size_t payload_size = 0;
	size_t pos = 0;

	pos = file_path.find_last_of("\\");
	file_name = file_path.substr(pos != std::string::npos ? pos + 1 : 0, file_path.size());

	if (file_name.size() < max_buffer) {
		content = StreamHandler::read_bytes_from_file(file_path);
		if (content.second > 0) {
			attempts--;
			crc = StreamHandler::crc32(content.first, content.second);
			enc_content = symmetric.encrypt(content.first, content.second);
			payload_size = payload.load(uuid, file_name, enc_content.first, enc_content.second);
			request.load(uuid, RQ1103, &payload, payload_size);
			return;
		}
		throw std::exception("non existing file\n");
	}
	throw std::exception("file name size exceeds the requirement of 255 bytes\n");
}

void SocketHandler::checksum_auth(Request& request) {
	printf("client transmitted file successfully\n");
	request.load(uuid, RQ1104, nullptr,0);
}

void SocketHandler::checksum_auth_failed(Request& request) {
	printf("sending file again\n");
	request.load(uuid, RQ1105, nullptr, 0);
}

void SocketHandler::failed_to_transmit_file(Request& request) {
	printf("fatal error ");
	request.load(uuid, RQ1106, nullptr, 0);
}

ushort SocketHandler::checksum(Response& response) {
	if (memcmp((byte*)&crc[0], (byte*)&response.response_2103.crc, sizeof(uint)) == 0)
		return RQ1104;
	else if (attempts)
		return RQ1105;
	return RQ1106;
}

void SocketHandler::initialize() {
	Response response;
	Request request;
	ushort code;
	std::vector<std::string> file;

	file = StreamHandler::read_from_file("transfer.info");
	if (file.size() > 2) {

		size_t split = file[0].find(':');
		if (split != std::string::npos) {
			host = file[0].substr(0, split);
			port = file[0].substr(split + 1, file[0].size() - split);
		}

		if (client.connect(host, port) != SOCK::ERR_CONNECT) {
			name = file[1];
			file_path = file[2];

			code = initialize_communication(request);

			while (code) {
				try {
					request_function req_func = request_map[code];
					if (req_func)
						(this->*req_func)(request);
					else
						throw std::exception("unrecognized request");

					client.send(request);
					if (code != RQ1105) {
						client.receive(response);
						if (response.header.code != RS2104 && request.header.code != RQ1106) {
							response_function res_func = response_map[response.header.code];
							if (res_func)
								code = (this->*res_func)(response);
							else
								throw std::exception("unrecognized response");
						}
						else
							break;
					}
					else
						code = RQ1103;
					request.clear();
				}
				catch (std::exception& error) {
					failed_to_transmit_file(request);
					printf(error.what());
					client.send(request);
					break;
				}
			}
			client.disconnect();
		}
	}
}