#pragma once
#include <iostream>
#include <cstdint>
#include <unordered_map>
#include "Cryptowrapper.h"

constexpr uint32_t	name_size		= 0xff;
constexpr uint32_t	aes_key_size	= 0x10;
constexpr uint32_t	public_key_size = 0xA0;
constexpr uint32_t	uuid_size		= 0x10;
constexpr uint32_t	version			= 0x03;

typedef uint8_t		byte;
typedef uint32_t	uint;
typedef uint16_t	ushort;

enum Packet {
	RQ1100 = 1100,
	RQ1101 = 1101,
	RQ1102 = 1102,
	RQ1103 = 1103,
	RQ1104 = 1104,
	RQ1105 = 1105,
	RQ1106 = 1106,
	RS2100 = 2100,
	RS2101 = 2101,
	RS2102 = 2102,
	RS2103 = 2103,
	RS2104 = 2104
};

#pragma pack(push,1)
struct request_1100 {
	byte				name[name_size];
public:
	uint load(std::string& name);
};

struct request_1101 {
	byte				name[name_size];
	byte				public_key[public_key_size];
	uint load(std::string&, std::string&);
};

struct request_1103 {
	byte				uuid[uuid_size];
	uint				content_size;
	byte				file_name[name_size];
	byte*				content;
	uint load(std::string&, std::string&, byte*, uint);
};

struct Request {
	struct {
		byte			uuid[uuid_size];
		byte			version;
		ushort			code;
		uint			payload_size;
	}header;

	union {
		/// payloads 1104,1105,1106 will be represented with generic messages
		request_1100	p1100;
		request_1101	p1101;
		request_1103	p1103;
	};
	void clear();
	std::pair<byte*, size_t> pack();
	void load(std::string&, uint , void* ,size_t);
};

struct Response {
	struct {
		byte			version;
		ushort			code;
		uint			payload_size;
	}header;
	
	union {
		/// payloads 2101,2104,2100 will be represented with generic messages
		struct {
			byte		uuid[uuid_size];
		}response_2100;

		struct {
			byte		uuid[uuid_size];
			byte*		aes_key;
		}response_2102;

		struct {
			byte		uuid[uuid_size];
			uint		content_size;
			byte		file_name[name_size];
			uint		crc;
		}response_2103;
	};
	void clear();
	void unpack(byte*, size_t);
};
#pragma pack(pop)
