#include "Cryptowrapper.h"
Asymmetric::Asymmetric() {}
Asymmetric::~Asymmetric() {}

std::string Asymmetric::decrypt(byte* cipher,size_t size) {
	std::string data;
	try {
		StringSource ss(private_key, true);
		RSA::PrivateKey prvkey;
		prvkey.Load(ss);
		RSAES_OAEP_SHA_Decryptor dec(prvkey);
		StringSource as(cipher, size, true,
			new PK_DecryptorFilter(rnd, dec,
				new StringSink(data)
			)
		);
	}
	catch (CryptoPP::Exception& e) {
		std::cout << e.what() << std::endl;
	}
	return data;
}

std::pair<std::string, std::string> Asymmetric::generate_random_keys() {
	try {
		RSA::PrivateKey prvkey;
		prvkey.GenerateRandomWithKeySize(rnd, Asymmetric::bkey_size);
		RSA::PublicKey pubkey(prvkey);
		StringSink ssprivate(private_key);
		prvkey.Save(ssprivate);
		StringSink sspublic(public_key);
		pubkey.Save(sspublic);

	}
	catch (CryptoPP::Exception& e) {
		std::cout << e.what() << std::endl;
	}
	return { public_key,private_key };
}

Symmetric::Symmetric() {
	memset(iv, 0, key_size);
	memset(key, 0, key_size);
	
}
Symmetric::~Symmetric() {}

std::pair<byte*,size_t> Symmetric::encrypt(byte* data,size_t size) {
	std::string c;
	byte* cipher = nullptr;
	AES::Encryption aes(key, key_size);
	CBC_Mode_ExternalCipher::Encryption cbc(aes, iv);
	StreamTransformationFilter stf(cbc, 
		new StringSink(c)
	);
	stf.Put(data, size);
	stf.MessageEnd();
	cipher = new byte[c.size()];
	memcpy_s(cipher, c.size(), &c[0], c.size());
	delete[] data;

	return {cipher,c.size()};
}

void Symmetric::load_key(std::string& aes_key) {
	memcpy_s(key, key_size, &aes_key[0], aes_key.size());
}
