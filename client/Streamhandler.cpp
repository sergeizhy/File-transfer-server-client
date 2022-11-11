#include "Streamhandler.h"

std::string StreamHandler::encode_base_64(std::string& raw) {
	std::string encoded;
	StringSource ss(raw, true,
		new Base64Encoder(
			new StringSink(encoded)
		) 
	); 
	return encoded;
}

std::string StreamHandler::decode_base_64(const std::string& encdoded) {
	std::string decoded;
	StringSource ss(encdoded, true,
		new Base64Decoder(
			new StringSink(decoded)
		)
	);
	return decoded;	
}

std::string StreamHandler::encode_hex(std::string& bytes) {
	std::string encoded;
	StringSource ss(bytes, true,
		new HexEncoder(
			new StringSink(encoded),
			true,
			2,
			" "
		) 
	);
	return encoded;
}

std::string StreamHandler::decode_hex(std::string& encoded) {
	std::string decoded;
	StringSource ss(encoded, true,
		new HexDecoder(
			new StringSink(decoded)
		)
	);
	return decoded;
}

std::string StreamHandler::crc32(byte* bytes, size_t size) {
	std::string digest;
	CRC32 crc;
	StringSource ss(bytes,size, true,
		new HashFilter(crc,
			new StringSink(digest)
		)
	);
	return digest;
}

bool StreamHandler::file_exists(const std::string file) {
	struct stat info;
	return stat(file.c_str(), &info) == 0;
}

std::pair<unsigned char*,size_t> StreamHandler::read_bytes_from_file(const std::string& file) {

	char* stream = nullptr;
	size_t size = 0;

	if (file_exists(file)) {
		std::ifstream fp(file.c_str(), std::ios::ate | std::ios::binary);
		if (fp.is_open()) {
			size = fp.tellg();
			fp.seekg(0, fp.beg);
			stream = new char[size];
			memset(stream, NULL, size);
			fp.read(stream, size);
			fp.close();
		}
	}
	return { (unsigned char*)stream , size};
}

std::vector<std::string> StreamHandler::read_from_file(const std::string& file) {

	std::vector<std::string> lines;
	std::ifstream fp(file.c_str());
	std::string line;
	if (fp.is_open()) {
		while (std::getline(fp, line))
			if (line.size())
				lines.push_back(line);
		fp.close();
	}
	return lines;
}

bool StreamHandler::write_to_file(const std::string& file, const std::string& data, byte mode) {
	std::fstream fp(file.c_str(), std::ios::out | mode);
	if (fp.is_open()) {		
		fp << data << '\n';
		fp.close();
		return true;
	}
	return false;
}

bool StreamHandler::write_to_file(const std::string& file, const std::vector<std::string>& data, byte mode) {
	std::fstream fp(file.c_str(), std::ios::out | mode);
	if (fp.is_open()) {
		for (std::string line : data) 
			fp << line << '\n';
		fp.close();
		return true;
	}
	return false;
}


