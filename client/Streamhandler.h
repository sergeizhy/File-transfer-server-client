#pragma once
#include <iostream>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/crc.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <fstream>

using namespace CryptoPP;

class StreamHandler {
public:
	static bool write_to_file(const std::string& , const std::string& ,byte );
	static bool write_to_file(const std::string&, const std::vector<std::string>&, byte);
	static std::vector <std::string> read_from_file(const std::string& );
	static std::pair<unsigned char*, size_t> read_bytes_from_file(const std::string&);
	static std::string encode_base_64(std::string& );
	static std::string decode_base_64(const std::string&);
	static std::string decode_hex(std::string& );
	static std::string encode_hex(std::string&);
	static bool file_exists(const std::string);
	static std::string crc32(byte*, size_t );
};