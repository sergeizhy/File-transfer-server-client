#pragma once
#include <iostream>
#include <string>
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>

using namespace				CryptoPP;

class Asymmetric {
public:
	static constexpr auto	key_size = 0x80;
	static constexpr auto	bkey_size = 0x400;
private:
	std::string				private_key;
	std::string				public_key;
	AutoSeededRandomPool	rnd;
public:
	Asymmetric();
	~Asymmetric();
	std::string decrypt(byte* cipher, size_t size);
	std::pair<std::string, std::string> generate_random_keys();
};
class Symmetric {
public:
	static constexpr auto	key_size = 0x10;
private:
	byte					key[key_size];
	byte					iv[key_size];
public:
	Symmetric();
	~Symmetric();
	std::pair<byte*, size_t> encrypt(byte*,size_t);
	std::string decrypt(byte*,size_t);
	void load_key(std::string&);
};
