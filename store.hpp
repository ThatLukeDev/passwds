#ifndef org_tld_passwds_store
#define org_tld_passwds_store

#include <fstream>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <functional>

#include "hash.hpp"
#include "aes.hpp"

class data {
public:
	unsigned char* content;
	unsigned int size = 0;

	data() { }

	data(unsigned int _size) {
		content = (unsigned char*)malloc(_size);
		size = _size;
	}

	data(unsigned char* _content, unsigned int _size) {
		content = _content;
		size = _size;
	}

	data(char* _content) {
		content = (unsigned char*)_content;
		size = sizeof(_content);
	}
};

namespace iosecure {
	std::string salt = "^$_aB%TTxm)|0/(dTL2)p!mM";

	std::string filename;
	std::string master;
	std::string contents;

	int getFiletype(char* _filename) {
		struct stat sb;
		if (stat(_filename, &sb) == -1) return -1; // no file
		if (sb.st_mode & S_IFDIR) return 1; // directory
		return 0; // file
	}

	int checkPasswd() {
		std::ifstream sr;
		sr.open(filename, std::ios::binary);

		if (sr.fail()) {
			return -1;
		}

		char* checksum = (char*)malloc(32);
		sr.read(checksum, 32);
		sr.close();

		char* hash = sha256::hashX1000((master+salt).c_str());

		bool eq = true;
		for (int i = 0; i < 32; i++) {
			if (*(checksum+i) != *(hash+i)) {
				eq = false;
			}
		}

		if (!eq) {
			return -1;
		}
		return 0;
	}

	int open(char* _filename) {
		filename = std::string(_filename);

		int type = getFiletype((char*)filename.c_str());

		switch (type) {
			case 1:
				return -1;
				break;
			case 0:
				std::ifstream sr;
				sr.open(filename, std::ios::binary);

				sr.seekg(0, std::ios::end);
				if (sr.tellg() < 5)
					return -1;
				data ciphered = data((int)sr.tellg()-32);
				sr.seekg(32);
				sr.read((char*)ciphered.content, ciphered.size);

				if (ciphered.size > 0) {
					char* raw = aes256::decrypt(ciphered.content, ciphered.size, sha256::hash(master.c_str()));
					contents = std::string(raw);
				}

				sr.close();
				free(ciphered.content);
				break;
		}

		return 0;
	}

	int close() {
		std::ofstream sw;
		sw.open(filename, std::ios::binary);

		char* hash = sha256::hashX1000((master+salt).c_str());

		data raw = data(std::ceil(contents.length()/16.0)*16+32);
		memcpy(raw.content, hash, 32);
		raw.content += 32;
		raw.size -= 32;
		strcpy((char*)raw.content, contents.c_str());

		if (contents.length() > 0) {
			char* ciphered = aes256::encrypt((void*)contents.c_str(), contents.size(), sha256::hash(master.c_str()));
			memcpy(raw.content, ciphered, std::ceil(contents.length()/16.0)*16);
		}

		raw.content -= 32;
		raw.size += 32;

		sw.write((char*)raw.content, raw.size);

		free(raw.content);

		sw.close();

		return 0;
	}
}

#endif
