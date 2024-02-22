#ifndef org_tld_passwds_store
#define org_tld_passwds_store

#include <fstream>
#include <sys/stat.h>
#include <string>
#include <functional>

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
	std::string filename;
	std::string master;
	std::string contents;

	std::hash<std::string> stringHash;

	int getFiletype(char* _filename) {
		struct stat sb;
		if (stat(_filename, &sb) == -1) return -1; // no file
		if (sb.st_mode & S_IFDIR) return 1; // directory
		return 0; // file
	}

	data vernam(data a, data b) {
		data build = data(a.size);

		for (int i = 0; i < a.size; i++) {
			build.content[i] = a.content[i] ^ b.content[i % b.size];
		}

		return build;
	}

	void vernamSelf(data a, data b) {
		for (int i = 0; i < a.size; i++) {
			a.content[i] = a.content[i] ^ b.content[i % b.size];
		}
	}

	int checkPasswd() {
		std::ifstream sr;
		sr.open(filename, std::ios::binary);

		if (sr.fail()) {
			return -1;
		}

		size_t checksum;
		sr.read((char*)&checksum, 4);

		sr.close();

		size_t hash = stringHash(master);

		if (hash != checksum) {
			return -1;
		}

		return 0;
	}

	int open(char* _filename) {
		filename = std::string(_filename);

		int type = getFiletype((char*)filename.c_str());

		if (type == 1) {
			return -1;
		}
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
				data ciphered = data((int)sr.tellg()-4);
				sr.seekg(4);
				sr.read((char*)ciphered.content, ciphered.size);

				data raw = vernam(ciphered, data((char*)master.c_str()));
				contents = std::string((char*)raw.content);

				sr.close();
				free(ciphered.content);
				free(raw.content);
				break;
		}

		return 0;
	}

	int close() {
		std::ofstream sw;
		sw.open(filename, std::ios::binary);

		size_t hash = stringHash(master);

		data raw = data(contents.size()+4);
		raw.content += 4;
		raw.size -= 4;
		raw.content = (unsigned char*)contents.c_str();

		vernamSelf(raw, data((char*)master.c_str()));

		raw.content -= 4;
		raw.size += 4;

		sw.write((char*)raw.content, raw.size);

		free(raw.content);

		sw.close();

		return 0;
	}
}

#endif
