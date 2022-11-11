#include "Packet.h"

std::pair<byte*, size_t> Request::pack() {
	std::unordered_map<ushort, size_t> map = {
		{RQ1100, sizeof(request_1100)},
		{RQ1101, sizeof(request_1101)},
		{RQ1103, sizeof(request_1103) - sizeof(byte*)}
	};

	Request* req = this;
	byte* buffer = nullptr;
	size_t header_size = sizeof(req->header) + map[req->header.code];
	size_t buffer_size = sizeof(req->header) + req->header.payload_size;

	buffer = new byte[buffer_size];
	memset(buffer, 0, buffer_size);
	memcpy_s(buffer, buffer_size, &req->header, header_size);

	if (req->header.code == RQ1103)
		memcpy_s(buffer + header_size, buffer_size - header_size, req->p1103.content, req->p1103.content_size);

	return {buffer, buffer_size};
}

void Response::unpack(byte* bytes, size_t size) {
	std::unordered_map<ushort, size_t> map = {
		{RS2100, sizeof(response_2100)},
		{RS2102, sizeof(response_2102) - sizeof(byte*)},
		{RS2103, sizeof(response_2103)}
	};
	Response* response = this;
	byte* aes_key = nullptr;
	size_t key_size = 0;
	size_t offset = 0;
	size_t header_size = sizeof(response->header);
	
	memcpy_s(response, sizeof(*response), bytes, header_size);
	offset = header_size;
	memcpy_s((byte*)response + offset, sizeof(*response) - offset, bytes + offset, map[response->header.code]);
	offset += map[response->header.code];

	if (response->header.code == RS2102) {
		key_size = response->header.payload_size - map[response->header.code];
		aes_key = new byte[key_size];
		memcpy_s(aes_key, key_size, bytes + offset, size - offset);
		response->response_2102.aes_key = aes_key;
	}
}

void Response::clear() {
	memset(this, 0, sizeof(*this));
}

void Request::clear() {
	memset(this, 0, sizeof(*this));
}

void Request::load(std::string& uuid, uint code, void* payload,size_t payload_size) {
	std::unordered_map<uint, size_t> map = {
		{RQ1100, sizeof(request_1100)},
		{RQ1101, sizeof(request_1101)},
		{RQ1103, sizeof(request_1103)}
	};

	memcpy_s(header.uuid, sizeof(header.uuid), &uuid[0], uuid.size());
	header.version = version;
	header.code = code;
	header.payload_size = payload_size;
	if (payload) 
		memcpy_s((byte*)this + sizeof(header), map[code], payload, map[code]);

}

uint request_1100::load(std::string& name) {
	memset(this, 0, sizeof(*this));
	if (name.size() < name_size) {
		memcpy_s(this->name,sizeof(this->name),&name[0],name.size());
		return sizeof(request_1100);
	}
	throw std::exception("exception occurred on request_1100::load : name exceeds 255 byte");
}

uint request_1101::load(std::string& name, std::string& aes_key) {
	memset(this, 0, sizeof(*this));
	if (name.size() < name_size && aes_key.size() > 0) {
		memcpy_s(this->name, sizeof(this->name), &name[0], name.size());
		memcpy_s(this->public_key, sizeof(this->public_key), &aes_key[0], aes_key.size());
		return sizeof(request_1101);
	}
	throw std::exception("exception occurred on request_1101::load : name or aes key is incorrect");
}

uint request_1103::load(std::string& uuid, std::string& file_name,byte* content, uint content_size) {
	memset(this, 0, sizeof(*this));
	if (uuid.size() == uuid_size) {
		this->content_size = content_size;
		this->content = content;
		memcpy_s(this->uuid, sizeof(this->uuid), &uuid[0], uuid.size());
		memcpy_s(this->file_name, sizeof(this->file_name), &file_name[0], file_name.size());
		return sizeof(request_1103) + content_size - sizeof(byte*);
	}
	throw std::exception("exception occurred on request_1103::load : uuid size is incorrect");
}